#include "rockchip_mpp_stream.h"

#include <fcntl.h>
#include <linux/videodev2.h>
#include <poll.h>
#include <rk_mpi.h>
#include <rk_mpp_cfg.h>
#include <rk_venc_cmd.h>
#include <rk_venc_rc.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "air_recording_helper.hpp"
#include "openhd_rtp.h"
#include "openhd_spdlog.h"

namespace {
constexpr RK_U32 align16(RK_U32 value) { return (value + 15U) & ~15U; }

int retry_ioctl(int fd, unsigned long request, void* argument) {
  int result;
  do {
    result = ioctl(fd, request, argument);
  } while (result < 0 && errno == EINTR);
  return result;
}

class MpegTsMuxer {
 public:
  ~MpegTsMuxer() { close(); }

  bool open(const std::string& path, bool h265, int fps) {
    m_file = std::fopen(path.c_str(), "wb");
    if (!m_file) return false;
    m_h265 = h265;
    m_fps = fps > 0 ? fps : 30;
    m_cc_pat = m_cc_pmt = m_cc_video = 0;
    m_frame_index = 0;
    m_frames_since_psi = 0;
    return true;
  }

  bool is_open() const { return m_file != nullptr; }

  void close() {
    if (!m_file) return;
    std::fflush(m_file);
    std::fclose(m_file);
    m_file = nullptr;
  }

  void write_access_unit(const uint8_t* header, size_t header_len,
                         const uint8_t* au, size_t au_len, bool keyframe) {
    if (!m_file || !au || !au_len) return;
    const uint64_t pts =
        static_cast<uint64_t>(m_frame_index) * 90000ULL / static_cast<uint64_t>(m_fps);
    ++m_frame_index;
    if (keyframe || m_frames_since_psi >= 20) {
      write_pat();
      write_pmt();
      m_frames_since_psi = 0;
    } else {
      ++m_frames_since_psi;
    }
    write_pes(header, header_len, au, au_len, pts, keyframe);
  }

 private:
  static constexpr int kPatPid = 0x0000;
  static constexpr int kPmtPid = 0x1000;
  static constexpr int kVideoPid = 0x0100;

  static uint32_t crc32_mpeg(const uint8_t* data, size_t len) {
    uint32_t crc = 0xFFFFFFFFU;
    for (size_t i = 0; i < len; ++i) {
      crc ^= static_cast<uint32_t>(data[i]) << 24;
      for (int b = 0; b < 8; ++b)
        crc = (crc & 0x80000000U) ? (crc << 1) ^ 0x04C11DB7U : (crc << 1);
    }
    return crc;
  }

  static void write_pts_field(uint8_t* p, uint8_t prefix, uint64_t pts) {
    p[0] = (prefix << 4) | (((pts >> 30) & 0x7) << 1) | 0x1;
    p[1] = (pts >> 22) & 0xFF;
    p[2] = (((pts >> 15) & 0x7F) << 1) | 0x1;
    p[3] = (pts >> 7) & 0xFF;
    p[4] = ((pts & 0x7F) << 1) | 0x1;
  }

  void write_ts(const uint8_t* packet) { std::fwrite(packet, 1, 188, m_file); }

  void write_psi(int pid, int& cc, const uint8_t* section, size_t len) {
    uint8_t pkt[188];
    std::memset(pkt, 0xFF, sizeof(pkt));
    pkt[0] = 0x47;
    pkt[1] = 0x40 | ((pid >> 8) & 0x1F);
    pkt[2] = pid & 0xFF;
    pkt[3] = 0x10 | (cc & 0x0F);
    cc = (cc + 1) & 0x0F;
    pkt[4] = 0x00;
    std::memcpy(pkt + 5, section, len);
    write_ts(pkt);
  }

  void write_pat() {
    uint8_t s[16];
    size_t n = 0;
    s[n++] = 0x00;                 // table_id (PAT)
    s[n++] = 0xB0;                 // section_syntax_indicator + length hi
    s[n++] = 0x0D;                 // section_length = 13
    s[n++] = 0x00; s[n++] = 0x01;  // transport_stream_id
    s[n++] = 0xC1;                 // version 0, current_next = 1
    s[n++] = 0x00;                 // section_number
    s[n++] = 0x00;                 // last_section_number
    s[n++] = 0x00; s[n++] = 0x01;  // program_number 1
    s[n++] = 0xE0 | ((kPmtPid >> 8) & 0x1F);
    s[n++] = kPmtPid & 0xFF;
    const uint32_t crc = crc32_mpeg(s, n);
    s[n++] = (crc >> 24) & 0xFF; s[n++] = (crc >> 16) & 0xFF;
    s[n++] = (crc >> 8) & 0xFF;  s[n++] = crc & 0xFF;
    write_psi(kPatPid, m_cc_pat, s, n);
  }

  void write_pmt() {
    uint8_t s[24];
    size_t n = 0;
    s[n++] = 0x02;                 // table_id (PMT)
    s[n++] = 0xB0;                 // section_syntax_indicator + length hi
    s[n++] = 0x12;                 // section_length = 18
    s[n++] = 0x00; s[n++] = 0x01;  // program_number
    s[n++] = 0xC1;                 // version 0, current_next = 1
    s[n++] = 0x00;                 // section_number
    s[n++] = 0x00;                 // last_section_number
    s[n++] = 0xE0 | ((kVideoPid >> 8) & 0x1F);  // PCR_PID
    s[n++] = kVideoPid & 0xFF;
    s[n++] = 0xF0; s[n++] = 0x00;  // program_info_length = 0
    s[n++] = m_h265 ? 0x24 : 0x1B; // stream_type (HEVC / AVC)
    s[n++] = 0xE0 | ((kVideoPid >> 8) & 0x1F);
    s[n++] = kVideoPid & 0xFF;
    s[n++] = 0xF0; s[n++] = 0x00;  // ES_info_length = 0
    const uint32_t crc = crc32_mpeg(s, n);
    s[n++] = (crc >> 24) & 0xFF; s[n++] = (crc >> 16) & 0xFF;
    s[n++] = (crc >> 8) & 0xFF;  s[n++] = crc & 0xFF;
    write_psi(kPmtPid, m_cc_pmt, s, n);
  }

  void write_pes(const uint8_t* header, size_t header_len, const uint8_t* au,
                 size_t au_len, uint64_t pts, bool keyframe) {
    m_pes.clear();
    m_pes.insert(m_pes.end(), {0x00, 0x00, 0x01, 0xE0});
    m_pes.insert(m_pes.end(), {0x00, 0x00});
    m_pes.insert(m_pes.end(), {0x80, 0x80, 0x05});
    uint8_t pts5[5];
    write_pts_field(pts5, 0x2, pts);
    m_pes.insert(m_pes.end(), pts5, pts5 + 5);
    if (header && header_len)
      m_pes.insert(m_pes.end(), header, header + header_len);
    m_pes.insert(m_pes.end(), au, au + au_len);

    size_t pos = 0;
    bool first = true;
    while (pos < m_pes.size()) {
      uint8_t pkt[188];
      size_t p = 0;
      pkt[p++] = 0x47;
      pkt[p++] = (first ? 0x40 : 0x00) | ((kVideoPid >> 8) & 0x1F);
      pkt[p++] = kVideoPid & 0xFF;
      const size_t remaining = m_pes.size() - pos;
      const bool want_pcr = first;
      if (want_pcr || remaining < 184) {
        const size_t af_content = 1 + (want_pcr ? 6 : 0);
        const size_t max_payload = 184 - 1 - af_content;
        const size_t payload = remaining < max_payload ? remaining : max_payload;
        const size_t stuffing = max_payload - payload;
        pkt[p++] = 0x30 | (m_cc_video & 0x0F);
        m_cc_video = (m_cc_video + 1) & 0x0F;
        pkt[p++] = static_cast<uint8_t>(af_content + stuffing);
        uint8_t flags = 0x00;
        if (keyframe && first) flags |= 0x40;
        if (want_pcr) flags |= 0x10;
        pkt[p++] = flags;
        if (want_pcr) {
          const uint64_t base = pts;
          pkt[p++] = (base >> 25) & 0xFF;
          pkt[p++] = (base >> 17) & 0xFF;
          pkt[p++] = (base >> 9) & 0xFF;
          pkt[p++] = (base >> 1) & 0xFF;
          pkt[p++] = ((base & 1) << 7) | 0x7E;
          pkt[p++] = 0x00;
        }
        std::memset(pkt + p, 0xFF, stuffing);
        p += stuffing;
        std::memcpy(pkt + p, m_pes.data() + pos, payload);
        p += payload;
        pos += payload;
      } else {
        pkt[p++] = 0x10 | (m_cc_video & 0x0F);
        m_cc_video = (m_cc_video + 1) & 0x0F;
        std::memcpy(pkt + p, m_pes.data() + pos, 184);
        p += 184;
        pos += 184;
      }
      write_ts(pkt);
      first = false;
    }
  }

  std::FILE* m_file = nullptr;
  bool m_h265 = false;
  int m_fps = 30;
  int m_cc_pat = 0, m_cc_pmt = 0, m_cc_video = 0;
  uint64_t m_frame_index = 0;
  int m_frames_since_psi = 0;
  std::vector<uint8_t> m_pes;
};
}

class RockchipMppStream::Impl {
 public:
  struct CaptureBuffer {
    void* data = nullptr;
    size_t size = 0;
  };

  Impl(RockchipMppStream& owner, std::shared_ptr<spdlog::logger> log)
      : owner(owner), log(std::move(log)) {
    const auto& settings = owner.m_camera_holder->get_settings();
    bitrate_kbits = settings.h26x_bitrate_kbits;
    requested_bitrate_kbits = settings.h26x_bitrate_kbits;
    qp_min = settings.qp_min;
    qp_max = settings.qp_max;
    update_roi_snapshot();
    update_recording_snapshot();
    update_debug_snapshot();
    rtp = std::make_shared<openhd::RTPHelper>(
        settings.streamed_video_format.videoCodec == VideoCodec::H265);
    rtp->set_out_cb([this](auto fragments) {
      if (!this->owner.m_output_cb || fragments.empty()) return;
      const int loss_percent = debug_packet_loss_percent.load();
      const int keyframe_loss_percent =
          transmitting_keyframe.load() ? debug_keyframe_loss_percent.load() : 0;
      if (loss_percent > 0 || keyframe_loss_percent > 0) {
        fragments.erase(
            std::remove_if(fragments.begin(), fragments.end(),
                           [this, loss_percent,
                            keyframe_loss_percent](const auto&) {
                             bool drop = false;
                             if (loss_percent > 0) {
                               packet_loss_accumulator += loss_percent;
                               if (packet_loss_accumulator >= 100) {
                                 packet_loss_accumulator -= 100;
                                 drop = true;
                               }
                             }
                             if (keyframe_loss_percent > 0) {
                               keyframe_loss_accumulator +=
                                   keyframe_loss_percent;
                               if (keyframe_loss_accumulator >= 100) {
                                 keyframe_loss_accumulator -= 100;
                                 drop = true;
                               }
                             }
                             return drop;
                           }),
            fragments.end());
        if (fragments.empty()) return;
      }
      const auto& s = this->owner.m_camera_holder->get_settings();
      openhd::FragmentedVideoFrame frame{
          std::move(fragments), std::chrono::steady_clock::now(),
          s.enable_ultra_secure_encryption, nullptr,
          intra_refresh_enable.load(), false};
      this->owner.m_output_cb(
          this->owner.m_camera_holder->get_camera().index, frame);
    });
  }

  ~Impl() { stop(); }

  bool init_mpp() {
    const auto& s = owner.m_camera_holder->get_settings();
    const auto& f = s.streamed_video_format;
    width = f.width;
    height = f.height;
    hor_stride = align16(width);
    ver_stride = align16(height);
    coding = f.videoCodec == VideoCodec::H265 ? MPP_VIDEO_CodingHEVC
                                               : MPP_VIDEO_CodingAVC;
    if (mpp_create(&ctx, &mpi) || mpp_init(ctx, MPP_CTX_ENC, coding)) {
      log->error("Cannot initialize native Rockchip MPP encoder");
      return false;
    }
    if (mpp_enc_cfg_init(&cfg) || mpi->control(ctx, MPP_ENC_GET_CFG, cfg)) {
      log->error("Cannot allocate native MPP encoder configuration");
      return false;
    }
    mpp_enc_cfg_set_s32(cfg, "codec:type", coding);
    mpp_enc_cfg_set_s32(cfg, "prep:width", width);
    mpp_enc_cfg_set_s32(cfg, "prep:height", height);
    mpp_enc_cfg_set_s32(cfg, "prep:hor_stride", hor_stride);
    mpp_enc_cfg_set_s32(cfg, "prep:ver_stride", ver_stride);
    mpp_enc_cfg_set_s32(cfg, "prep:format", MPP_FMT_YUV420SP);
    mpp_enc_cfg_set_s32(cfg, "prep:rotation", s.camera_rotation_degree / 90);
    mpp_enc_cfg_set_s32(cfg, "rc:mode", MPP_ENC_RC_MODE_CBR);
    mpp_enc_cfg_set_s32(cfg, "rc:fps_in_flex", 0);
    mpp_enc_cfg_set_s32(cfg, "rc:fps_in_num", f.framerate);
    mpp_enc_cfg_set_s32(cfg, "rc:fps_in_denom", 1);
    mpp_enc_cfg_set_s32(cfg, "rc:fps_out_flex", 0);
    mpp_enc_cfg_set_s32(cfg, "rc:fps_out_num", f.framerate);
    mpp_enc_cfg_set_s32(cfg, "rc:fps_out_denom", 1);
    // Keep IDR cadence independent from cyclic intra refresh. In particular,
    // sparse refresh must not turn every completed sweep into a full IDR.
    mpp_enc_cfg_set_s32(cfg, "rc:gop",
                        std::max(1, s.h26x_keyframe_interval));
    if (coding == MPP_VIDEO_CodingAVC) {
      mpp_enc_cfg_set_s32(cfg, "h264:profile", 100);
      mpp_enc_cfg_set_s32(cfg, "h264:level", 42);
      mpp_enc_cfg_set_s32(cfg, "h264:cabac_en", 1);
    }
    apply_chroma_qp_offset(cfg);
    apply_rate_control();
    if (mpi->control(ctx, MPP_ENC_SET_CFG, cfg)) {
      log->error("MPP rejected encoder configuration");
      return false;
    }
    if (mpp_buffer_group_get_internal(&group,
                                      MPP_BUFFER_TYPE_DRM |
                                          MPP_BUFFER_FLAGS_CACHABLE) ||
        mpp_buffer_get(group, &input_buffer,
                       hor_stride * ver_stride * 3 / 2)) {
      log->error("Cannot allocate MPP input buffer");
      return false;
    }
    emit_codec_header();
    const auto cam_index = owner.m_camera_holder->get_camera().index;
    openhd::LinkActionHandler::CamInfo cam_info{
        true,
        static_cast<uint8_t>(cam_index),
        static_cast<uint8_t>(owner.m_camera_holder->get_camera().camera_type),
        CameraStream::CAM_STATUS_RESTARTING,
        static_cast<uint8_t>(s.air_recording),
        static_cast<uint8_t>(video_codec_to_int(f.videoCodec)),
        static_cast<uint16_t>(s.h26x_bitrate_kbits),
        static_cast<uint16_t>(s.h26x_bitrate_kbits),
        static_cast<uint8_t>(s.h26x_keyframe_interval),
        static_cast<uint16_t>(width), static_cast<uint16_t>(height),
        static_cast<uint16_t>(f.framerate), 0, 0, 0,
        static_cast<uint8_t>(s.qp_max),
        static_cast<uint8_t>(s.qp_min)};
    openhd::LinkActionHandler::instance().set_cam_info(cam_index, cam_info);
    openhd::LinkActionHandler::instance().set_cam_info_supports_variable_bitrate(
        cam_index, true);
    return true;
  }

  bool init_record_encoder() {
    const auto& f = owner.m_camera_holder->get_settings().streamed_video_format;
    if (mpp_create(&record_ctx, &record_mpi) ||
        mpp_init(record_ctx, MPP_CTX_ENC, coding) ||
        mpp_enc_cfg_init(&record_cfg) ||
        record_mpi->control(record_ctx, MPP_ENC_GET_CFG, record_cfg)) {
      log->error("Cannot initialize the high-quality MPP recording channel");
      destroy_record_encoder();
      return false;
    }
    mpp_enc_cfg_set_s32(record_cfg, "codec:type", coding);
    mpp_enc_cfg_set_s32(record_cfg, "prep:width", width);
    mpp_enc_cfg_set_s32(record_cfg, "prep:height", height);
    mpp_enc_cfg_set_s32(record_cfg, "prep:hor_stride", hor_stride);
    mpp_enc_cfg_set_s32(record_cfg, "prep:ver_stride", ver_stride);
    mpp_enc_cfg_set_s32(record_cfg, "prep:format", MPP_FMT_YUV420SP);
    mpp_enc_cfg_set_s32(record_cfg, "rc:mode", MPP_ENC_RC_MODE_CBR);
    mpp_enc_cfg_set_s32(record_cfg, "rc:fps_in_flex", 0);
    mpp_enc_cfg_set_s32(record_cfg, "rc:fps_in_num", f.framerate);
    mpp_enc_cfg_set_s32(record_cfg, "rc:fps_in_denom", 1);
    mpp_enc_cfg_set_s32(record_cfg, "rc:fps_out_flex", 0);
    mpp_enc_cfg_set_s32(record_cfg, "rc:fps_out_num", f.framerate);
    mpp_enc_cfg_set_s32(record_cfg, "rc:fps_out_denom", 1);
    mpp_enc_cfg_set_s32(record_cfg, "rc:gop",
                        std::max(1, f.framerate * 2));
    if (coding == MPP_VIDEO_CodingAVC) {
      mpp_enc_cfg_set_s32(record_cfg, "h264:profile", 100);
      mpp_enc_cfg_set_s32(record_cfg, "h264:level", 42);
      mpp_enc_cfg_set_s32(record_cfg, "h264:cabac_en", 1);
    }
    apply_chroma_qp_offset(record_cfg);
    apply_record_rate_control();
    if (record_mpi->control(record_ctx, MPP_ENC_SET_CFG, record_cfg)) {
      log->error("MPP rejected the recording-channel configuration");
      destroy_record_encoder();
      return false;
    }
    return true;
  }

  void apply_chroma_qp_offset(MppEncCfg target) {
    const char* cb = coding == MPP_VIDEO_CodingHEVC ? "h265:cb_qp_offset"
                                                    : "h264:cb_qp_offset";
    const char* cr = coding == MPP_VIDEO_CodingHEVC ? "h265:cr_qp_offset"
                                                    : "h264:cr_qp_offset";
    mpp_enc_cfg_set_s32(target, cb, -6);
    mpp_enc_cfg_set_s32(target, cr, -6);
  }

  void apply_rate_control() {
    const int bps = std::max(1000, bitrate_kbits.load()) * 1000;
    // RV1126's MPP rate controller otherwise keeps its default three-second
    // statistics window, which makes interactive bitrate changes appear to be
    // ignored for simple synthetic scenes.
    mpp_enc_cfg_set_s32(cfg, "rc:stats_time", 1);
    mpp_enc_cfg_set_s32(cfg, "rc:drop_mode", 0);
    mpp_enc_cfg_set_s32(cfg, "rc:bps_target", bps);
    mpp_enc_cfg_set_s32(cfg, "rc:bps_min", bps * 9 / 10);
    mpp_enc_cfg_set_s32(cfg, "rc:bps_max", bps * 11 / 10);
    mpp_enc_cfg_set_s32(cfg, "rc:qp_init", -1);
    mpp_enc_cfg_set_s32(cfg, "rc:qp_min", qp_min.load());
    mpp_enc_cfg_set_s32(cfg, "rc:qp_max", qp_max.load());
    mpp_enc_cfg_set_s32(cfg, "rc:qp_min_i", qp_min.load());
    mpp_enc_cfg_set_s32(cfg, "rc:qp_max_i", qp_max.load());
    // RV1126 MPP native intra-refresh. Mode 2 is the OpenHD sparse-block mode
    // and refreshes only refresh_num raster-ordered macroblocks per frame.
    mpp_enc_cfg_set_s32(cfg, "rc:refresh_en", intra_refresh_enable.load());
    mpp_enc_cfg_set_s32(cfg, "rc:refresh_mode", intra_refresh_mode.load());
    mpp_enc_cfg_set_s32(cfg, "rc:refresh_num", intra_refresh_num.load());
  }

  void apply_record_rate_control() {
    const int bps = std::max(5000, record_bitrate_kbits.load()) * 1000;
    mpp_enc_cfg_set_s32(record_cfg, "rc:stats_time", 1);
    mpp_enc_cfg_set_s32(record_cfg, "rc:drop_mode", 0);
    mpp_enc_cfg_set_s32(record_cfg, "rc:bps_target", bps);
    mpp_enc_cfg_set_s32(record_cfg, "rc:bps_min", bps * 9 / 10);
    mpp_enc_cfg_set_s32(record_cfg, "rc:bps_max", bps * 11 / 10);
    mpp_enc_cfg_set_s32(record_cfg, "rc:qp_init", -1);
    mpp_enc_cfg_set_s32(record_cfg, "rc:qp_min", record_qp_min.load());
    mpp_enc_cfg_set_s32(record_cfg, "rc:qp_max", record_qp_max.load());
    mpp_enc_cfg_set_s32(record_cfg, "rc:qp_min_i", record_qp_min.load());
    mpp_enc_cfg_set_s32(record_cfg, "rc:qp_max_i", record_qp_max.load());
    mpp_enc_cfg_set_s32(record_cfg, "rc:refresh_en", intra_refresh_enable.load());
    mpp_enc_cfg_set_s32(record_cfg, "rc:refresh_mode", intra_refresh_mode.load());
    mpp_enc_cfg_set_s32(record_cfg, "rc:refresh_num", intra_refresh_num.load());
  }

  void emit_codec_header() {
    MppBuffer packet_buffer = nullptr;
    MppPacket packet = nullptr;
    const size_t size = std::max<size_t>(width * height, 64 * 1024);
    if (mpp_buffer_get(group, &packet_buffer, size) ||
        mpp_packet_init_with_buffer(&packet, packet_buffer)) return;
    mpp_packet_set_length(packet, 0);
    if (!mpi->control(ctx, MPP_ENC_GET_HDR_SYNC, packet)) {
      const auto* data = static_cast<const uint8_t*>(mpp_packet_get_pos(packet));
      const auto length = mpp_packet_get_length(packet);
      if (data && length) rtp->feed_multiple_nalu(data, static_cast<int>(length));
    }
    mpp_packet_deinit(&packet);
    mpp_buffer_put(packet_buffer);
  }

  bool setup_record_file() {
    const auto& f = owner.m_camera_holder->get_settings().streamed_video_format;
    recording_filename = openhd::video::create_unused_recording_filename(".ts");
    if (!record_muxer.open(recording_filename,
                           f.videoCodec == VideoCodec::H265, f.framerate)) {
      log->error("Cannot open MPP recording file {}: {}", recording_filename,
                 std::strerror(errno));
      return false;
    }
    return true;
  }

  void emit_record_codec_header() {
    record_header.clear();
    MppBuffer packet_buffer = nullptr;
    MppPacket packet = nullptr;
    const size_t size = std::max<size_t>(width * height, 64 * 1024);
    if (mpp_buffer_get(group, &packet_buffer, size) ||
        mpp_packet_init_with_buffer(&packet, packet_buffer)) return;
    mpp_packet_set_length(packet, 0);
    if (!record_mpi->control(record_ctx, MPP_ENC_GET_HDR_SYNC, packet)) {
      const auto* data = static_cast<const uint8_t*>(mpp_packet_get_pos(packet));
      const auto length = mpp_packet_get_length(packet);
      if (data && length) record_header.assign(data, data + length);
    }
    mpp_packet_deinit(&packet);
    mpp_buffer_put(packet_buffer);
  }

  bool is_keyframe_au(const uint8_t* au, size_t len) const {
    for (size_t i = 0; i + 4 < len; ++i) {
      if (au[i] != 0x00 || au[i + 1] != 0x00) continue;
      size_t nal = 0;
      if (au[i + 2] == 0x01)
        nal = i + 3;
      else if (au[i + 2] == 0x00 && au[i + 3] == 0x01)
        nal = i + 4;
      else
        continue;
      if (nal >= len) break;
      if (coding == MPP_VIDEO_CodingHEVC) {
        const int type = (au[nal] >> 1) & 0x3F;
        if (type >= 16 && type <= 23) return true;
      } else {
        if ((au[nal] & 0x1F) == 5) return true;
      }
    }
    return false;
  }

  bool start_recording() {
    if (record_ctx) return true;
    if (!init_record_encoder() || !setup_record_file()) {
      stop_recording();
      return false;
    }
    emit_record_codec_header();
    log->info("High-quality MPP recording started: {} kbit/s, QP {}-{}, {}",
              record_bitrate_kbits.load(), record_qp_min.load(),
              record_qp_max.load(), recording_filename);
    return true;
  }

  void destroy_record_encoder() {
    if (record_cfg) mpp_enc_cfg_deinit(record_cfg);
    if (record_ctx) mpp_destroy(record_ctx);
    record_cfg = nullptr;
    record_ctx = nullptr;
    record_mpi = nullptr;
  }

  void stop_recording() {
    record_muxer.close();
    record_au.clear();
    record_header.clear();
    destroy_record_encoder();
    if (!recording_filename.empty()) {
      log->info("High-quality MPP recording stopped: {}", recording_filename);
      recording_filename.clear();
    }
  }

  bool recording_requested() const {
    const int mode = owner.m_camera_holder->get_settings().air_recording;
    return mode == AIR_RECORDING_ON ||
           (mode == AIR_RECORDING_AUTO_ARM_DISARM && armed.load());
  }

  bool setup_capture() {
    const auto& camera = owner.m_camera_holder->get_camera();
    if (camera.requires_rockchip1126_mpp_testsrc_pipeline()) {
      synthetic_capture = true;
      synthetic_frame.resize(static_cast<size_t>(width) * height * 3 / 2);
      return true;
    }

    // The RV1126 CSI pipeline exposes the ISP's NV12 capture output on the
    // main-path node. /dev/video0 is the rkaiisp control endpoint and does not
    // implement V4L2 video capture ioctls.
    constexpr const char* capture_device = "/dev/video13";
    capture_fd = open(capture_device, O_RDWR | O_NONBLOCK | O_CLOEXEC);
    if (capture_fd < 0) {
      log->error("Cannot open {}: {}", capture_device, std::strerror(errno));
      return false;
    }
    v4l2_format format{};
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    format.fmt.pix_mp.width = width;
    format.fmt.pix_mp.height = height;
    format.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV12;
    format.fmt.pix_mp.field = V4L2_FIELD_NONE;
    format.fmt.pix_mp.num_planes = 1;
    if (retry_ioctl(capture_fd, VIDIOC_S_FMT, &format) < 0 ||
        format.fmt.pix_mp.pixelformat != V4L2_PIX_FMT_NV12) {
      log->error("{} cannot provide NV12 {}x{}: {}", capture_device, width,
                 height, std::strerror(errno));
      return false;
    }

    // S_FMT selects the resolution, but the IMX415 exposes several modes with
    // the same native size (30/60/90 fps). Propagate the requested rate through
    // the ISP pipeline so the sensor driver's s_frame_interval callback can
    // select the matching timing table.
    const int requested_fps = std::max(
        1, owner.m_camera_holder->get_settings().streamed_video_format.framerate);
    v4l2_streamparm streamparm{};
    streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    streamparm.parm.capture.timeperframe.numerator = 1;
    streamparm.parm.capture.timeperframe.denominator = requested_fps;
    if (retry_ioctl(capture_fd, VIDIOC_S_PARM, &streamparm) < 0) {
      log->warn("{} rejected requested capture rate {} fps: {}", capture_device,
                requested_fps, std::strerror(errno));
    } else {
      const auto& actual = streamparm.parm.capture.timeperframe;
      const double actual_fps = actual.numerator
                                    ? static_cast<double>(actual.denominator) /
                                          actual.numerator
                                    : 0.0;
      log->info("{} capture rate requested {} fps, selected {:.2f} fps",
                capture_device, requested_fps, actual_fps);
    }
    capture_num_planes = format.fmt.pix_mp.num_planes;
    if (capture_num_planes < 1 || capture_num_planes > VIDEO_MAX_PLANES) {
      log->error("{} reported unsupported plane count {}", capture_device,
                 capture_num_planes);
      return false;
    }

    capture_stride = std::max<uint32_t>(format.fmt.pix_mp.plane_fmt[0].bytesperline,
                                        width);
    // The luma plane's height is aligned by the driver independently of
    // `height` (e.g. 540 -> 544), so the interleaved UV plane does not begin at
    // stride*height. Reading it there pulls 4 rows of zeroed luma padding into
    // the chroma and paints a green bar across the top of the frame. Derive the
    // real luma height from the driver-reported sizeimage (NV12 = stride * yh *
    // 3/2), falling back to a 16-row alignment.
    const uint32_t sizeimage = format.fmt.pix_mp.plane_fmt[0].sizeimage;
    uint32_t luma_rows =
        sizeimage ? (sizeimage / capture_stride) * 2 / 3 : align16(height);
    if (luma_rows < height) luma_rows = align16(height);
    capture_uv_offset = static_cast<size_t>(capture_stride) * luma_rows;
    v4l2_requestbuffers request{};
    request.count = 4;
    request.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    request.memory = V4L2_MEMORY_MMAP;
    if (retry_ioctl(capture_fd, VIDIOC_REQBUFS, &request) < 0 ||
        request.count < 2) {
      log->error("Cannot allocate V4L2 capture buffers: {}",
                 std::strerror(errno));
      return false;
    }
    capture_buffers.resize(request.count);
    for (uint32_t index = 0; index < request.count; ++index) {
      v4l2_buffer buffer{};
      v4l2_plane planes[VIDEO_MAX_PLANES]{};
      buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
      buffer.memory = V4L2_MEMORY_MMAP;
      buffer.index = index;
      buffer.m.planes = planes;
      buffer.length = capture_num_planes;
      if (retry_ioctl(capture_fd, VIDIOC_QUERYBUF, &buffer) < 0) return false;
      capture_buffers[index].size = planes[0].length;
      capture_buffers[index].data =
          mmap(nullptr, planes[0].length, PROT_READ | PROT_WRITE, MAP_SHARED,
               capture_fd, planes[0].m.mem_offset);
      if (capture_buffers[index].data == MAP_FAILED) return false;
      if (retry_ioctl(capture_fd, VIDIOC_QBUF, &buffer) < 0) return false;
    }
    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    if (retry_ioctl(capture_fd, VIDIOC_STREAMON, &type) < 0) {
      log->error("Cannot start V4L2 capture: {}", std::strerror(errno));
      return false;
    }
    capture_streaming = true;
    return true;
  }

  void fill_synthetic_frame(uint64_t frame_index) {
    auto* y = synthetic_frame.data();
    auto* uv = y + static_cast<size_t>(width) * height;
    for (RK_U32 row = 0; row < height; ++row) {
      for (RK_U32 column = 0; column < width; ++column) {
        const bool bar = ((column + frame_index * 4) / 64) % 2;
        y[static_cast<size_t>(row) * width + column] =
            static_cast<uint8_t>((row * 180 / std::max<RK_U32>(1, height)) +
                                 (bar ? 48 : 16));
      }
    }
    for (RK_U32 row = 0; row < height / 2; ++row) {
      for (RK_U32 column = 0; column < width; column += 2) {
        uv[static_cast<size_t>(row) * width + column] =
            static_cast<uint8_t>(96 + (frame_index / 2) % 64);
        uv[static_cast<size_t>(row) * width + column + 1] =
            static_cast<uint8_t>(160 - (frame_index / 3) % 64);
      }
    }
  }

  uint32_t next_noise_random() {
    noise_prng ^= noise_prng << 13;
    noise_prng ^= noise_prng >> 17;
    noise_prng ^= noise_prng << 5;
    return noise_prng;
  }

  void add_noise(uint8_t* destination) {
    const int strength = debug_noise_percent.load();
    if (!destination || strength <= 0) return;
    const int amplitude = std::max(1, strength * 64 / 100);
    auto alter = [this, amplitude](uint8_t value) {
      const int delta = static_cast<int>(next_noise_random() %
                                         (amplitude * 2 + 1)) - amplitude;
      return static_cast<uint8_t>(std::clamp<int>(value + delta, 0, 255));
    };
    for (RK_U32 row = 0; row < height; ++row) {
      auto* line = destination + row * hor_stride;
      for (RK_U32 column = 0; column < width; ++column)
        line[column] = alter(line[column]);
    }
    auto* uv = destination + hor_stride * ver_stride;
    for (RK_U32 row = 0; row < height / 2; ++row) {
      auto* line = uv + row * hor_stride;
      for (RK_U32 column = 0; column < width; ++column)
        line[column] = alter(line[column]);
    }
  }

  void update_bitrate_sweep() {
    if (!debug_bitrate_sweep.load()) return;
    const int minimum = debug_bitrate_min_kbits.load();
    const int maximum = std::max(minimum, debug_bitrate_max_kbits.load());
    const int period_ms =
        std::max(2, debug_bitrate_period_seconds.load()) * 1000;
    const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::steady_clock::now().time_since_epoch())
                            .count();
    const auto elapsed_ms = now_ms - sweep_epoch_ms.load();
    const double phase = static_cast<double>(elapsed_ms % period_ms) / period_ms;
    const double triangle = phase < 0.5 ? phase * 2.0 : (1.0 - phase) * 2.0;
    int target = minimum + static_cast<int>((maximum - minimum) * triangle);
    target = ((target + 50) / 100) * 100;
    if (target != bitrate_kbits.load()) {
      bitrate_kbits = target;
      rate_dirty = true;
    }
  }

  bool capture_one_frame() {
    pollfd descriptor{capture_fd, POLLIN, 0};
    const int poll_result = poll(&descriptor, 1, 100);
    if (poll_result <= 0) return poll_result == 0 || errno == EINTR;
    v4l2_buffer buffer{};
    v4l2_plane planes[VIDEO_MAX_PLANES]{};
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.m.planes = planes;
    buffer.length = capture_num_planes;
    if (retry_ioctl(capture_fd, VIDIOC_DQBUF, &buffer) < 0) {
      return errno == EAGAIN;
    }
    if (buffer.index < capture_buffers.size()) {
      const auto& mapped = capture_buffers[buffer.index];
      encode_nv12(static_cast<const uint8_t*>(mapped.data),
                  std::min<size_t>(planes[0].bytesused, mapped.size));
    }
    if (retry_ioctl(capture_fd, VIDIOC_QBUF, &buffer) < 0) {
      log->error("Cannot requeue V4L2 buffer: {}", std::strerror(errno));
      return false;
    }
    return true;
  }

  void run() {
    while (running &&
           !owner.m_camera_holder->get_settings().enable_streaming) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    if (!running) return;
    openhd::LinkActionHandler::instance().set_cam_info_status(
        owner.m_camera_holder->get_camera().index,
        CameraStream::CAM_STATUS_RESTARTING);
    if (!init_mpp() || !setup_capture()) {
      running = false;
      cleanup();
      return;
    }
    log->info("Native MPP encoder active: {}x{} ROI is runtime adjustable",
              width, height);
    openhd::LinkActionHandler::instance().set_cam_info_status(
        owner.m_camera_holder->get_camera().index,
        CameraStream::CAM_STATUS_STREAMING);
    const int fps = std::max(
        1, owner.m_camera_holder->get_settings().streamed_video_format.framerate);
    const auto frame_period = std::chrono::microseconds(1000000 / fps);
    auto next_frame = std::chrono::steady_clock::now();
    uint64_t synthetic_index = 0;
    while (running) {
      if (synthetic_capture) {
        fill_synthetic_frame(synthetic_index++);
        encode_nv12(synthetic_frame.data(), synthetic_frame.size());
        next_frame += frame_period;
        std::this_thread::sleep_until(next_frame);
      } else if (!capture_one_frame()) {
        break;
      }
      const auto now = std::chrono::steady_clock::now();
      if (now - last_space_check >= std::chrono::seconds(5)) {
        last_space_check = now;
        owner.m_camera_holder->check_remaining_space_air_recording(false);
      }
    }
    cleanup();
  }

  void encode_nv12(const uint8_t* source, size_t source_size) {
    const size_t src_stride = capture_stride ? capture_stride : width;
    const size_t uv_offset = capture_uv_offset
                                 ? capture_uv_offset
                                 : src_stride * static_cast<size_t>(height);
    const size_t required_size =
        uv_offset + src_stride * static_cast<size_t>(height / 2);
    if (!source || source_size < required_size) return;
    update_bitrate_sweep();
    if (rate_dirty.exchange(false)) {
      apply_rate_control();
      if (mpi->control(ctx, MPP_ENC_SET_CFG, cfg))
        log->warn("MPP rejected a dynamic bitrate/QP update");
    }
    mpp_buffer_sync_begin(input_buffer);
    auto* destination = static_cast<uint8_t*>(mpp_buffer_get_ptr(input_buffer));
    if (!destination) {
      mpp_buffer_sync_end(input_buffer);
      return;
    }
    for (RK_U32 row = 0; row < height; ++row)
      std::memcpy(destination + row * hor_stride,
                  source + static_cast<size_t>(row) * src_stride, width);
    const uint8_t* source_uv = source + uv_offset;
    uint8_t* destination_uv = destination + hor_stride * ver_stride;
    for (RK_U32 row = 0; row < height / 2; ++row)
      std::memcpy(destination_uv + row * hor_stride,
                  source_uv + static_cast<size_t>(row) * src_stride, width);
    add_noise(destination);
    mpp_buffer_sync_end(input_buffer);

    encode_context(ctx, mpi, true);
    if (recording_requested()) {
      if (start_recording()) {
        if (record_rate_dirty.exchange(false)) {
          apply_record_rate_control();
          if (record_mpi->control(record_ctx, MPP_ENC_SET_CFG, record_cfg))
            log->warn("MPP rejected a dynamic recording-quality update");
        }
        encode_context(record_ctx, record_mpi, false);
      }
    } else if (record_ctx || record_muxer.is_open()) {
      stop_recording();
    }
  }

  void encode_context(MppCtx encoder_ctx, MppApi* encoder_mpi,
                      bool transmit) {
    MppFrame frame = nullptr;
    if (mpp_frame_init(&frame)) return;
    mpp_frame_set_width(frame, width);
    mpp_frame_set_height(frame, height);
    mpp_frame_set_hor_stride(frame, hor_stride);
    mpp_frame_set_ver_stride(frame, ver_stride);
    mpp_frame_set_fmt(frame, MPP_FMT_YUV420SP);
    mpp_frame_set_buffer(frame, input_buffer);
    MppEncROIRegion region{};
    MppEncROICfg roi_cfg{};
    if (roi_enable.load()) {
      configure_roi(region, roi_cfg);
      mpp_meta_set_ptr(mpp_frame_get_meta(frame), KEY_ROI_DATA, &roi_cfg);
    }
    if (transmit && force_keyframe_pending.exchange(false) &&
        encoder_mpi->control(encoder_ctx, MPP_ENC_SET_IDR_FRAME, nullptr)) {
      log->warn("MPP rejected the force-keyframe request");
    }
    if (encoder_mpi->encode_put_frame(encoder_ctx, frame)) {
      log->warn("MPP {} channel failed to accept an input frame",
                transmit ? "transmit" : "recording");
      mpp_frame_deinit(&frame);
      return;
    }
    mpp_frame_deinit(&frame);
    RK_U32 end_of_image = 0;
    size_t transmit_frame_bytes = 0;
    do {
      MppPacket packet = nullptr;
      if (encoder_mpi->encode_get_packet(encoder_ctx, &packet) || !packet)
        break;
      const auto* data = static_cast<const uint8_t*>(mpp_packet_get_pos(packet));
      const size_t length = mpp_packet_get_length(packet);
      end_of_image = !mpp_packet_is_partition(packet) || mpp_packet_is_eoi(packet);
      if (data && length) {
        if (transmit) {
          RK_S32 is_intra = 0;
          MppMeta meta = mpp_packet_get_meta(packet);
          if (meta)
            mpp_meta_get_s32(meta, KEY_OUTPUT_INTRA, &is_intra);
          if (is_intra || is_keyframe_au(data, length))
            transmitting_keyframe = true;
          transmit_frame_bytes += length;
          perf_window_bytes += length;
          rtp->feed_multiple_nalu(data, static_cast<int>(length));
        } else {
          record_au.insert(record_au.end(), data, data + length);
        }
      }
      mpp_packet_deinit(&packet);
      if (transmit && end_of_image) transmitting_keyframe = false;
    } while (!end_of_image);
    if (!transmit && !record_au.empty()) {
      const bool keyframe = is_keyframe_au(record_au.data(), record_au.size());
      record_muxer.write_access_unit(
          keyframe ? record_header.data() : nullptr,
          keyframe ? record_header.size() : 0, record_au.data(),
          record_au.size(), keyframe);
      record_au.clear();
    }
    if (transmit) {
      // Pace filler against wall-clock time, not configured FPS. The hardware
      // may output fewer frames than requested; per-frame padding would then
      // undershoot the requested link bitrate by that same ratio.
      const auto now = std::chrono::steady_clock::now();
      const auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                                  now - perf_window_start)
                                  .count();
      const size_t target_window_bytes =
          static_cast<size_t>(std::max(1000, bitrate_kbits.load())) * 1000ULL *
          static_cast<size_t>(std::max<int64_t>(0, elapsed_ms)) / 8ULL / 1000ULL;
      if (perf_window_bytes < target_window_bytes) {
        const auto padding = emit_bitrate_padding(target_window_bytes -
                                                  perf_window_bytes);
        transmit_frame_bytes += padding;
        perf_window_bytes += padding;
      }
      ++perf_window_frames;
      const auto elapsed = now - perf_window_start;
      if (elapsed >= std::chrono::seconds(1)) {
        const auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                                    elapsed)
                                    .count();
        const auto bitrate_bps = static_cast<uint32_t>(
            perf_window_bytes * 8ULL * 1000ULL /
            static_cast<uint64_t>(std::max<int64_t>(1, elapsed_ms)));
        const auto fps = static_cast<uint16_t>(
            perf_window_frames * 1000ULL /
            static_cast<uint64_t>(std::max<int64_t>(1, elapsed_ms)));
        openhd::LinkActionHandler::instance().set_cam_info_perf(
            owner.m_camera_holder->get_camera().index, bitrate_bps, fps);
        perf_window_bytes = 0;
        perf_window_frames = 0;
        perf_window_start = now;
      }
    }
  }

  size_t emit_bitrate_padding(size_t bytes) {
    // H.264/H.265 filler NAL units are ignored by decoders but consume the
    // same RTP/link bandwidth as ordinary video. This makes a requested MPP
    // bitrate testable with low-complexity synthetic scenes without polluting
    // the high-quality recording stream.
    size_t emitted = 0;
    const bool h265 = coding == MPP_VIDEO_CodingHEVC;
    while (bytes >= (h265 ? 6U : 5U)) {
      const size_t payload = std::min<size_t>(bytes - (h265 ? 6U : 5U), 60000);
      std::vector<uint8_t> filler;
      filler.reserve(payload + (h265 ? 6U : 5U));
      filler.insert(filler.end(), {0, 0, 0, 1});
      if (h265)
        filler.insert(filler.end(), {static_cast<uint8_t>(38U << 1), 1});
      else
        filler.push_back(0x0c);  // H.264 filler_data NAL unit
      filler.insert(filler.end(), payload, 0);
      // rbsp_trailing_bits terminates the filler NAL cleanly.
      filler.back() = 0x80;
      rtp->feed_multiple_nalu(filler.data(), static_cast<int>(filler.size()));
      emitted += filler.size();
      if (filler.size() >= bytes) break;
      bytes -= filler.size();
    }
    return emitted;
  }

  void configure_roi(MppEncROIRegion& region, MppEncROICfg& roi_cfg) {
    auto pct = [](RK_U32 extent, int percentage) {
      return align16(extent * static_cast<RK_U32>(percentage) / 100U);
    };
    region.x = std::min(pct(width, roi_x.load()), align16(width) - 16);
    region.y = std::min(pct(height, roi_y.load()), align16(height) - 16);
    region.w = std::max<RK_U32>(16, pct(width, roi_width.load()));
    region.h = std::max<RK_U32>(16, pct(height, roi_height.load()));
    region.w = std::min<RK_U32>(region.w, align16(width) - region.x);
    region.h = std::min<RK_U32>(region.h, align16(height) - region.y);
    region.quality = roi_quality.load();
    region.abs_qp_en = 0;
    roi_cfg.number = 1;
    roi_cfg.regions = &region;
  }

  void update_roi_snapshot() {
    const auto& s = owner.m_camera_holder->get_settings();
    // These are the primary MPP resilience features; keep them enabled even
    // if an older persisted profile contains a false/missing value.
    roi_enable = s.mpp_roi_enable;
    roi_x = s.mpp_roi_x_percent;
    roi_y = s.mpp_roi_y_percent;
    roi_width = s.mpp_roi_width_percent;
    roi_height = s.mpp_roi_height_percent;
    roi_quality = s.mpp_roi_quality;
    intra_refresh_enable = s.mpp_intra_refresh_enable;
    intra_refresh_mode = s.mpp_intra_refresh_mode;
    intra_refresh_num = s.mpp_intra_refresh_num;
    rate_dirty = true;
  }

  void update_recording_snapshot() {
    const auto& s = owner.m_camera_holder->get_settings();
    record_bitrate_kbits = s.mpp_record_bitrate_kbits;
    record_qp_min = s.mpp_record_qp_min;
    record_qp_max = s.mpp_record_qp_max;
    record_rate_dirty = true;
  }

  void update_debug_snapshot() {
    const auto& s = owner.m_camera_holder->get_settings();
    debug_noise_percent = s.mpp_debug_noise_percent;
    debug_packet_loss_percent = s.mpp_debug_packet_loss_percent;
    debug_keyframe_loss_percent = s.mpp_debug_keyframe_loss_percent;
    debug_bitrate_min_kbits = s.mpp_debug_bitrate_min_kbits;
    debug_bitrate_max_kbits = s.mpp_debug_bitrate_max_kbits;
    debug_bitrate_period_seconds = s.mpp_debug_bitrate_period_seconds;
    const bool was_sweeping = debug_bitrate_sweep.exchange(
        s.mpp_debug_bitrate_sweep);
    sweep_epoch_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::steady_clock::now().time_since_epoch())
                         .count();
    if (!s.mpp_debug_bitrate_sweep) {
      bitrate_kbits = requested_bitrate_kbits.load();
      rate_dirty = true;
    } else if (!was_sweeping) {
      bitrate_kbits = debug_bitrate_min_kbits.load();
      rate_dirty = true;
    }
    log->info("MPP video test mode: noise {}%, RTP loss {}%, keyframe loss "
              "{}%, sweep {} "
              "({}-{} kbit/s over {}s)",
              debug_noise_percent.load(), debug_packet_loss_percent.load(),
              debug_keyframe_loss_percent.load(),
              debug_bitrate_sweep.load() ? "on" : "off",
              debug_bitrate_min_kbits.load(), debug_bitrate_max_kbits.load(),
              debug_bitrate_period_seconds.load());
  }

  void cleanup() {
    stop_recording();
    if (capture_streaming) {
      v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
      retry_ioctl(capture_fd, VIDIOC_STREAMOFF, &type);
    }
    for (auto& buffer : capture_buffers) {
      if (buffer.data && buffer.data != MAP_FAILED)
        munmap(buffer.data, buffer.size);
    }
    capture_buffers.clear();
    if (capture_fd >= 0) close(capture_fd);
    capture_fd = -1;
    capture_stride = 0;
    capture_uv_offset = 0;
    capture_num_planes = 1;
    capture_streaming = false;
    synthetic_capture = false;
    synthetic_frame.clear();
    if (input_buffer) mpp_buffer_put(input_buffer);
    if (group) mpp_buffer_group_put(group);
    if (cfg) mpp_enc_cfg_deinit(cfg);
    if (ctx) mpp_destroy(ctx);
    input_buffer = nullptr;
    group = nullptr;
    cfg = nullptr;
    ctx = nullptr;
    mpi = nullptr;
  }

  void stop() {
    running = false;
    if (thread.joinable()) thread.join();
  }

  RockchipMppStream& owner;
  std::shared_ptr<spdlog::logger> log;
  std::shared_ptr<openhd::RTPHelper> rtp;
  std::thread thread;
  std::atomic_bool running{false};
  std::atomic_bool rate_dirty{false};
  std::atomic<int> bitrate_kbits{8000};
  std::atomic<int> requested_bitrate_kbits{8000};
  std::atomic<int> qp_min{5};
  std::atomic<int> qp_max{51};
  std::atomic_bool roi_enable{false};
  std::atomic<int> roi_x{25};
  std::atomic<int> roi_y{25};
  std::atomic<int> roi_width{50};
  std::atomic<int> roi_height{50};
  std::atomic<int> roi_quality{-8};
  std::atomic_bool intra_refresh_enable{false};
  std::atomic<int> intra_refresh_mode{2};
  std::atomic<int> intra_refresh_num{8};
  std::atomic_bool armed{false};
  std::atomic_bool record_rate_dirty{false};
  std::atomic<int> record_bitrate_kbits{40000};
  std::atomic<int> record_qp_min{4};
  std::atomic<int> record_qp_max{28};
  std::atomic<int> debug_noise_percent{0};
  std::atomic<int> debug_packet_loss_percent{0};
  std::atomic<int> debug_keyframe_loss_percent{0};
  std::atomic_bool transmitting_keyframe{false};
  std::atomic_bool force_keyframe_pending{false};
  std::atomic_bool debug_bitrate_sweep{false};
  std::atomic<int> debug_bitrate_min_kbits{2000};
  std::atomic<int> debug_bitrate_max_kbits{12000};
  std::atomic<int> debug_bitrate_period_seconds{10};
  int packet_loss_accumulator = 0;
  int keyframe_loss_accumulator = 0;
  uint32_t noise_prng = 0x4f484431U;
  std::atomic<int64_t> sweep_epoch_ms{0};
  uint64_t perf_window_bytes = 0;
  uint32_t perf_window_frames = 0;
  std::chrono::steady_clock::time_point perf_window_start =
      std::chrono::steady_clock::now();
  int capture_fd = -1;
  uint32_t capture_num_planes = 1;
  uint32_t capture_stride = 0;
  size_t capture_uv_offset = 0;
  bool capture_streaming = false;
  bool synthetic_capture = false;
  std::vector<CaptureBuffer> capture_buffers;
  std::vector<uint8_t> synthetic_frame;
  MppCtx ctx = nullptr;
  MppApi* mpi = nullptr;
  MppEncCfg cfg = nullptr;
  MppBufferGroup group = nullptr;
  MppBuffer input_buffer = nullptr;
  MppCtx record_ctx = nullptr;
  MppApi* record_mpi = nullptr;
  MppEncCfg record_cfg = nullptr;
  MpegTsMuxer record_muxer;
  std::vector<uint8_t> record_au;
  std::vector<uint8_t> record_header;
  std::string recording_filename;
  std::chrono::steady_clock::time_point last_space_check =
      std::chrono::steady_clock::now();
  MppCodingType coding = MPP_VIDEO_CodingAVC;
  RK_U32 width = 0, height = 0, hor_stride = 0, ver_stride = 0;
};

RockchipMppStream::RockchipMppStream(
    std::shared_ptr<CameraHolder> camera_holder,
    openhd::ON_ENCODE_FRAME_CB out_cb)
    : CameraStream(std::move(camera_holder), std::move(out_cb)),
      m_impl(std::make_unique<Impl>(
          *this, openhd::log::create_or_get("mpp_stream"))) {
  m_camera_holder->register_listener(
      [this]() { terminate_looping(); start_looping(); });
  m_camera_holder->register_video_bitrate_listener([this](int bitrate_kbits) {
    handle_change_bitrate_request({bitrate_kbits});
  });
  m_camera_holder->register_video_qp_listener([this](int min, int max) {
    m_impl->qp_min = min;
    m_impl->qp_max = max;
    m_impl->rate_dirty = true;
  });
  m_camera_holder->register_video_roi_listener(
      [this]() { m_impl->update_roi_snapshot(); });
  m_camera_holder->register_video_recording_listener(
      [this]() { m_impl->update_recording_snapshot(); });
  m_camera_holder->register_video_debug_listener(
      [this]() { m_impl->update_debug_snapshot(); });
  m_camera_holder->register_video_force_keyframe_listener(
      [this]() { m_impl->force_keyframe_pending = true; });
}

RockchipMppStream::~RockchipMppStream() {
  m_camera_holder->register_video_bitrate_listener(nullptr);
  m_camera_holder->register_video_qp_listener(nullptr);
  m_camera_holder->register_video_roi_listener(nullptr);
  m_camera_holder->register_video_recording_listener(nullptr);
  m_camera_holder->register_video_debug_listener(nullptr);
  m_camera_holder->register_video_force_keyframe_listener(nullptr);
  terminate_looping();
}

void RockchipMppStream::start_looping() {
  if (m_impl->running.exchange(true)) return;
  m_impl->thread = std::thread([this]() { m_impl->run(); });
}

void RockchipMppStream::terminate_looping() { m_impl->stop(); }

void RockchipMppStream::handle_change_bitrate_request(
    openhd::LinkActionHandler::LinkBitrateInformation lb) {
  m_impl->requested_bitrate_kbits = m_camera_holder->clamp_video_bitrate_kbits(
      lb.recommended_encoder_bitrate_kbits);
  openhd::LinkActionHandler::instance().set_cam_info_bitrate(
      m_camera_holder->get_camera().index,
      static_cast<uint16_t>(m_impl->requested_bitrate_kbits.load()));
  openhd::log::get_default()->info(
      "Camera{} native MPP bitrate request: {} kbit/s{}",
      m_camera_holder->get_camera().index,
      m_impl->requested_bitrate_kbits.load(),
      m_impl->debug_bitrate_sweep.load() ? " (sweep active)" : "");
  if (!m_impl->debug_bitrate_sweep.load()) {
    m_impl->bitrate_kbits = m_impl->requested_bitrate_kbits.load();
    m_impl->rate_dirty = true;
  }
}

void RockchipMppStream::handle_update_arming_state(bool armed) {
  m_impl->armed = armed;
}
