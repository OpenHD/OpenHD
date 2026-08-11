#include "rockchip_mpp_stream.h"

#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>
#include <gst/gst.h>
#include <rk_mpi.h>
#include <rk_mpp_cfg.h>
#include <rk_venc_cmd.h>
#include <rk_venc_rc.h>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <sstream>

#include "air_recording_helper.hpp"
#include "openhd_rtp.h"
#include "openhd_spdlog.h"

namespace {
constexpr RK_U32 align16(RK_U32 value) { return (value + 15U) & ~15U; }
}

class RockchipMppStream::Impl {
 public:
  Impl(RockchipMppStream& owner, std::shared_ptr<spdlog::logger> log)
      : owner(owner), log(std::move(log)) {
    const auto& settings = owner.m_camera_holder->get_settings();
    bitrate_kbits = settings.h26x_bitrate_kbits;
    qp_min = settings.qp_min;
    qp_max = settings.qp_max;
    update_roi_snapshot();
    update_recording_snapshot();
    rtp = std::make_shared<openhd::RTPHelper>(
        settings.streamed_video_format.videoCodec == VideoCodec::H265);
    rtp->set_out_cb([this](auto fragments) {
      if (!this->owner.m_output_cb || fragments.empty()) return;
      const auto& s = this->owner.m_camera_holder->get_settings();
      openhd::FragmentedVideoFrame frame{
          std::move(fragments), std::chrono::steady_clock::now(),
          s.enable_ultra_secure_encryption, nullptr,
          s.h26x_intra_refresh_type != -1, false};
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
    mpp_enc_cfg_set_s32(cfg, "rc:gop", std::max(1, s.h26x_keyframe_interval));
    if (coding == MPP_VIDEO_CodingAVC) {
      mpp_enc_cfg_set_s32(cfg, "h264:profile", 100);
      mpp_enc_cfg_set_s32(cfg, "h264:level", 42);
      mpp_enc_cfg_set_s32(cfg, "h264:cabac_en", 1);
    }
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
    apply_record_rate_control();
    if (record_mpi->control(record_ctx, MPP_ENC_SET_CFG, record_cfg)) {
      log->error("MPP rejected the recording-channel configuration");
      destroy_record_encoder();
      return false;
    }
    return true;
  }

  void apply_rate_control() {
    const int bps = std::max(1000, bitrate_kbits.load()) * 1000;
    mpp_enc_cfg_set_s32(cfg, "rc:bps_target", bps);
    mpp_enc_cfg_set_s32(cfg, "rc:bps_min", bps * 9 / 10);
    mpp_enc_cfg_set_s32(cfg, "rc:bps_max", bps * 11 / 10);
    mpp_enc_cfg_set_s32(cfg, "rc:qp_init", -1);
    mpp_enc_cfg_set_s32(cfg, "rc:qp_min", qp_min.load());
    mpp_enc_cfg_set_s32(cfg, "rc:qp_max", qp_max.load());
    mpp_enc_cfg_set_s32(cfg, "rc:qp_min_i", qp_min.load());
    mpp_enc_cfg_set_s32(cfg, "rc:qp_max_i", qp_max.load());
  }

  void apply_record_rate_control() {
    const int bps = std::max(5000, record_bitrate_kbits.load()) * 1000;
    mpp_enc_cfg_set_s32(record_cfg, "rc:bps_target", bps);
    mpp_enc_cfg_set_s32(record_cfg, "rc:bps_min", bps * 9 / 10);
    mpp_enc_cfg_set_s32(record_cfg, "rc:bps_max", bps * 11 / 10);
    mpp_enc_cfg_set_s32(record_cfg, "rc:qp_init", -1);
    mpp_enc_cfg_set_s32(record_cfg, "rc:qp_min", record_qp_min.load());
    mpp_enc_cfg_set_s32(record_cfg, "rc:qp_max", record_qp_max.load());
    mpp_enc_cfg_set_s32(record_cfg, "rc:qp_min_i", record_qp_min.load());
    mpp_enc_cfg_set_s32(record_cfg, "rc:qp_max_i", record_qp_max.load());
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

  bool setup_record_mux() {
    const auto codec = owner.m_camera_holder->get_settings()
                           .streamed_video_format.videoCodec;
    recording_filename = openhd::video::create_unused_recording_filename(".mkv");
    std::ostringstream text;
    text << "appsrc name=mpp_record_source is-live=true format=time block=true "
            "max-bytes=8388608 ! video/x-"
         << (codec == VideoCodec::H265 ? "h265" : "h264")
         << ",stream-format=byte-stream,alignment=au ! "
         << (codec == VideoCodec::H265 ? "h265parse" : "h264parse")
         << " config-interval=-1 ! matroskamux ! filesink location=\""
         << recording_filename << "\"";
    GError* error = nullptr;
    record_pipeline = gst_parse_launch(text.str().c_str(), &error);
    if (!record_pipeline) {
      log->error("Cannot create MPP recording muxer: {}",
                 error ? error->message : "unknown error");
      if (error) g_error_free(error);
      return false;
    }
    record_source = gst_bin_get_by_name(GST_BIN(record_pipeline),
                                        "mpp_record_source");
    if (!record_source ||
        gst_element_set_state(record_pipeline, GST_STATE_PLAYING) ==
            GST_STATE_CHANGE_FAILURE) {
      log->error("Cannot start MPP recording muxer");
      return false;
    }
    record_frame_index = 0;
    return true;
  }

  void push_record_data(const uint8_t* data, size_t length, bool header) {
    if (!record_source || !data || !length) return;
    GstBuffer* buffer = gst_buffer_new_allocate(nullptr, length, nullptr);
    if (!buffer) return;
    gst_buffer_fill(buffer, 0, data, length);
    if (!header) {
      const int fps = std::max(
          1, owner.m_camera_holder->get_settings().streamed_video_format.framerate);
      const GstClockTime duration = GST_SECOND / fps;
      GST_BUFFER_PTS(buffer) = record_frame_index * duration;
      GST_BUFFER_DTS(buffer) = GST_BUFFER_PTS(buffer);
      GST_BUFFER_DURATION(buffer) = duration;
    }
    const auto result =
        gst_app_src_push_buffer(GST_APP_SRC(record_source), buffer);
    if (result != GST_FLOW_OK) {
      log->warn("Recording muxer rejected MPP data ({})",
                static_cast<int>(result));
    }
  }

  void emit_record_codec_header() {
    MppBuffer packet_buffer = nullptr;
    MppPacket packet = nullptr;
    const size_t size = std::max<size_t>(width * height, 64 * 1024);
    if (mpp_buffer_get(group, &packet_buffer, size) ||
        mpp_packet_init_with_buffer(&packet, packet_buffer)) return;
    mpp_packet_set_length(packet, 0);
    if (!record_mpi->control(record_ctx, MPP_ENC_GET_HDR_SYNC, packet)) {
      push_record_data(
          static_cast<const uint8_t*>(mpp_packet_get_pos(packet)),
          mpp_packet_get_length(packet), true);
    }
    mpp_packet_deinit(&packet);
    mpp_buffer_put(packet_buffer);
  }

  bool start_recording() {
    if (record_ctx) return true;
    if (!init_record_encoder() || !setup_record_mux()) {
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
    if (record_source) gst_app_src_end_of_stream(GST_APP_SRC(record_source));
    if (record_pipeline) {
      GstBus* bus = gst_element_get_bus(record_pipeline);
      if (bus) {
        GstMessage* message = gst_bus_timed_pop_filtered(
            bus, 2 * GST_SECOND,
            static_cast<GstMessageType>(GST_MESSAGE_EOS | GST_MESSAGE_ERROR));
        if (message) gst_message_unref(message);
        gst_object_unref(bus);
      }
      gst_element_set_state(record_pipeline, GST_STATE_NULL);
    }
    if (record_source) gst_object_unref(record_source);
    if (record_pipeline) gst_object_unref(record_pipeline);
    record_source = nullptr;
    record_pipeline = nullptr;
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
    gst_init(nullptr, nullptr);
    const auto& camera = owner.m_camera_holder->get_camera();
    const auto& f = owner.m_camera_holder->get_settings().streamed_video_format;
    std::ostringstream pipeline_text;
    if (camera.requires_rockchip1126_mpp_testsrc_pipeline()) {
      pipeline_text << "videotestsrc is-live=true ! ";
    } else {
      pipeline_text << "v4l2src device=/dev/video0 ! ";
    }
    pipeline_text << "video/x-raw,format=NV12,width=" << f.width
                  << ",height=" << f.height << ",framerate=" << f.framerate
                  << "/1 ! appsink name=mpp_raw_sink sync=false max-buffers=2 "
                     "drop=true";
    GError* error = nullptr;
    pipeline = gst_parse_launch(pipeline_text.str().c_str(), &error);
    if (!pipeline) {
      log->error("Cannot create MPP capture pipeline: {}",
                 error ? error->message : "unknown error");
      if (error) g_error_free(error);
      return false;
    }
    sink = gst_bin_get_by_name(GST_BIN(pipeline), "mpp_raw_sink");
    if (!sink || gst_element_set_state(pipeline, GST_STATE_PLAYING) ==
                     GST_STATE_CHANGE_FAILURE) {
      log->error("Cannot start raw capture for native MPP encoder");
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
    while (running) {
      GstSample* sample = gst_app_sink_try_pull_sample(
          GST_APP_SINK(sink), 100 * GST_MSECOND);
      if (!sample) continue;
      GstBuffer* buffer = gst_sample_get_buffer(sample);
      GstMapInfo map{};
      if (buffer && gst_buffer_map(buffer, &map, GST_MAP_READ)) {
        encode_nv12(map.data, map.size);
        gst_buffer_unmap(buffer, &map);
      }
      gst_sample_unref(sample);
      const auto now = std::chrono::steady_clock::now();
      if (now - last_space_check >= std::chrono::seconds(5)) {
        last_space_check = now;
        owner.m_camera_holder->check_remaining_space_air_recording(false);
      }
    }
    cleanup();
  }

  void encode_nv12(const uint8_t* source, size_t source_size) {
    const size_t packed_size = static_cast<size_t>(width) * height * 3 / 2;
    if (!source || source_size < packed_size) return;
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
      std::memcpy(destination + row * hor_stride, source + row * width, width);
    const uint8_t* source_uv = source + width * height;
    uint8_t* destination_uv = destination + hor_stride * ver_stride;
    for (RK_U32 row = 0; row < height / 2; ++row)
      std::memcpy(destination_uv + row * hor_stride,
                  source_uv + row * width, width);
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
        ++record_frame_index;
      }
    } else if (record_ctx || record_pipeline) {
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
    if (encoder_mpi->encode_put_frame(encoder_ctx, frame)) {
      log->warn("MPP {} channel failed to accept an input frame",
                transmit ? "transmit" : "recording");
      mpp_frame_deinit(&frame);
      return;
    }
    mpp_frame_deinit(&frame);
    RK_U32 end_of_image = 0;
    do {
      MppPacket packet = nullptr;
      if (encoder_mpi->encode_get_packet(encoder_ctx, &packet) || !packet)
        break;
      const auto* data = static_cast<const uint8_t*>(mpp_packet_get_pos(packet));
      const size_t length = mpp_packet_get_length(packet);
      end_of_image = !mpp_packet_is_partition(packet) || mpp_packet_is_eoi(packet);
      if (data && length) {
        if (transmit)
          rtp->feed_multiple_nalu(data, static_cast<int>(length));
        else
          push_record_data(data, length, false);
      }
      mpp_packet_deinit(&packet);
    } while (!end_of_image);
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
    roi_enable = s.mpp_roi_enable;
    roi_x = s.mpp_roi_x_percent;
    roi_y = s.mpp_roi_y_percent;
    roi_width = s.mpp_roi_width_percent;
    roi_height = s.mpp_roi_height_percent;
    roi_quality = s.mpp_roi_quality;
  }

  void update_recording_snapshot() {
    const auto& s = owner.m_camera_holder->get_settings();
    record_bitrate_kbits = s.mpp_record_bitrate_kbits;
    record_qp_min = s.mpp_record_qp_min;
    record_qp_max = s.mpp_record_qp_max;
    record_rate_dirty = true;
  }

  void cleanup() {
    stop_recording();
    if (pipeline) gst_element_set_state(pipeline, GST_STATE_NULL);
    if (sink) gst_object_unref(sink);
    if (pipeline) gst_object_unref(pipeline);
    sink = nullptr;
    pipeline = nullptr;
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
  std::atomic<int> qp_min{5};
  std::atomic<int> qp_max{51};
  std::atomic_bool roi_enable{false};
  std::atomic<int> roi_x{25};
  std::atomic<int> roi_y{25};
  std::atomic<int> roi_width{50};
  std::atomic<int> roi_height{50};
  std::atomic<int> roi_quality{-8};
  std::atomic_bool armed{false};
  std::atomic_bool record_rate_dirty{false};
  std::atomic<int> record_bitrate_kbits{40000};
  std::atomic<int> record_qp_min{4};
  std::atomic<int> record_qp_max{28};
  GstElement* pipeline = nullptr;
  GstElement* sink = nullptr;
  MppCtx ctx = nullptr;
  MppApi* mpi = nullptr;
  MppEncCfg cfg = nullptr;
  MppBufferGroup group = nullptr;
  MppBuffer input_buffer = nullptr;
  MppCtx record_ctx = nullptr;
  MppApi* record_mpi = nullptr;
  MppEncCfg record_cfg = nullptr;
  GstElement* record_pipeline = nullptr;
  GstElement* record_source = nullptr;
  std::string recording_filename;
  uint64_t record_frame_index = 0;
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
}

RockchipMppStream::~RockchipMppStream() {
  m_camera_holder->register_video_bitrate_listener(nullptr);
  m_camera_holder->register_video_qp_listener(nullptr);
  m_camera_holder->register_video_roi_listener(nullptr);
  m_camera_holder->register_video_recording_listener(nullptr);
  terminate_looping();
}

void RockchipMppStream::start_looping() {
  if (m_impl->running.exchange(true)) return;
  m_impl->thread = std::thread([this]() { m_impl->run(); });
}

void RockchipMppStream::terminate_looping() { m_impl->stop(); }

void RockchipMppStream::handle_change_bitrate_request(
    openhd::LinkActionHandler::LinkBitrateInformation lb) {
  m_impl->bitrate_kbits = m_camera_holder->clamp_video_bitrate_kbits(
      lb.recommended_encoder_bitrate_kbits);
  m_impl->rate_dirty = true;
}

void RockchipMppStream::handle_update_arming_state(bool armed) {
  m_impl->armed = armed;
}
