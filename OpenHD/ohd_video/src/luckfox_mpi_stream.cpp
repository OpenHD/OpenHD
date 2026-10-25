// RV1103/RV1106 native camera: RKAIQ -> Rockit VI -> VENC -> OpenHD RTP.
// Implements the common MPP worker interface without linking GStreamer.
#include "rockchip_mpp_stream.h"

#include <rk_mpi_sys.h>
#include <rk_mpi_vi.h>
#include <rk_mpi_venc.h>
#include <rk_mpi_mb.h>
#include <rk_aiq_user_api2_sysctl.h>
#include <fcntl.h>
#include <linux/v4l2-subdev.h>
#include <linux/media-bus-format.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <stdexcept>
#include <vector>

#include "air_recording_helper.hpp"
#include "matroska_recorder.h"
#include "openhd_rtp.h"
#include "openhd_spdlog.h"
#include "rockchip_stream_utils.h"

class RockchipMppStream::Impl {
 public:
  using Clock = std::chrono::steady_clock;
  explicit Impl(RockchipMppStream& stream) : owner(stream) {
    const auto& s = owner.m_camera_holder->get_settings();
    bitrate = s.h26x_bitrate_kbits;
    qp_min = s.qp_min;
    qp_max = s.qp_max;
    orientation = (s.camera_rotation_degree / 90 << 2) | s.openhd_flip;
    record_mode = s.air_recording;
  }

  void check(RK_S32 result, const char* operation) {
    if (result != RK_SUCCESS)
      throw std::runtime_error(std::string(operation) + " failed: " + std::to_string(result));
  }

  void sensor_mode(int width, int height) {
    // Preserve the Pico MIS5001's existing binned/full readout selection.
    // Configure the sensor through its own subdevice, never an assumed index.
    for (int i = 0; i < 32; ++i) {
      const auto name = "v4l-subdev" + std::to_string(i);
      std::ifstream input("/sys/class/video4linux/" + name + "/name");
      std::string sensor;
      std::getline(input, sensor);
      if (sensor.find("mis5001") == std::string::npos) continue;
      const int fd = open(("/dev/" + name).c_str(), O_RDWR | O_CLOEXEC);
      if (fd < 0) throw std::runtime_error("Cannot open MIS5001 sensor");
      v4l2_subdev_format fmt{};
      fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
      fmt.format.width = width > 1296 || height > 968 ? 2592 : 1296;
      fmt.format.height = width > 1296 || height > 968 ? 1944 : 968;
      fmt.format.code = MEDIA_BUS_FMT_SGRBG10_1X10;
      const int result = ioctl(fd, VIDIOC_SUBDEV_S_FMT, &fmt);
      close(fd);
      if (result < 0) throw std::runtime_error("MIS5001 rejected capture mode");
      sensor_width = fmt.format.width;
      sensor_height = fmt.format.height;
      return;
    }
  }

  void setup() {
    const auto& s = owner.m_camera_holder->get_settings();
    const auto& f = s.streamed_video_format;
    width = f.width;
    height = f.height;
    fps = f.framerate;
    h265 = f.videoCodec == VideoCodec::H265;
    if (width <= 0 || height <= 0 || fps <= 0)
      throw std::runtime_error("Invalid Pico video format");
    sensor_mode(width, height);
    rk_aiq_static_info_t sensor{};
    check(rk_aiq_uapi2_sysctl_enumStaticMetasByPhyId(0, &sensor), "Enumerate sensor");
    aiq = rk_aiq_uapi2_sysctl_init(sensor.sensor_info.sensor_name,
                                  "/oem/usr/share/iqfiles", nullptr, nullptr);
    if (!aiq) throw std::runtime_error("RKAIQ sensor initialization failed");
    check(rk_aiq_uapi2_sysctl_prepare(aiq, sensor_width, sensor_height,
                                     RK_AIQ_WORKING_MODE_NORMAL), "Prepare RKAIQ");
    check(rk_aiq_uapi2_sysctl_start(aiq), "Start RKAIQ");
    aiq_started = true;
    check(RK_MPI_SYS_Init(), "Initialize MPI");
    system_started = true;
    VI_DEV_ATTR_S dev{};
    const auto configured = RK_MPI_VI_GetDevAttr(0, &dev);
    if (configured == RK_ERR_VI_NOT_CONFIG) {
      dev.enIntfMode = VI_MODE_MIPI_YUV420_NORMAL;
      check(RK_MPI_VI_SetDevAttr(0, &dev), "Configure VI device");
    } else check(configured, "Read VI device");
    check(RK_MPI_VI_EnableDev(0), "Enable VI device");
    vi_dev = true;
    VI_DEV_BIND_PIPE_S pipe{};
    pipe.u32Num = 1;
    pipe.PipeId[0] = 0;
    check(RK_MPI_VI_SetDevBindPipe(0, &pipe), "Bind VI pipe");
    VI_CHN_ATTR_S channel{};
    channel.stIspOpt.u32BufCount = 3;
    channel.stIspOpt.enMemoryType = VI_V4L2_MEMORY_TYPE_DMABUF;
    channel.stSize.u32Width = width;
    channel.stSize.u32Height = height;
    channel.enPixelFormat = RK_FMT_YUV420SP;
    channel.enCompressMode = COMPRESS_MODE_NONE;
    channel.stFrameRate.s32SrcFrameRate = -1;
    channel.stFrameRate.s32DstFrameRate = fps;
    check(RK_MPI_VI_SetChnAttr(0, 0, &channel), "Configure VI channel");
    check(RK_MPI_VI_EnableChn(0, 0), "Enable VI channel");
    vi_channel = true;
    VENC_CHN_ATTR_S encoder{};
    encoder.stVencAttr.enType = h265 ? RK_VIDEO_ID_HEVC : RK_VIDEO_ID_AVC;
    encoder.stVencAttr.enPixelFormat = RK_FMT_YUV420SP;
    encoder.stVencAttr.u32Profile = h265 ? 0 : 100;
    encoder.stVencAttr.u32PicWidth = encoder.stVencAttr.u32VirWidth = width;
    encoder.stVencAttr.u32PicHeight = encoder.stVencAttr.u32VirHeight = height;
    encoder.stVencAttr.u32StreamBufCnt = 3;
    encoder.stVencAttr.u32BufSize = std::max(width * height * 3 / 2, 256 * 1024);
    encoder.stRcAttr.enRcMode = h265 ? VENC_RC_MODE_H265CBR : VENC_RC_MODE_H264CBR;
    auto& rate = h265 ? encoder.stRcAttr.stH265Cbr : encoder.stRcAttr.stH264Cbr;
    rate.u32Gop = std::max(1, s.h26x_keyframe_interval);
    rate.u32BitRate = bitrate;
    rate.u32StatTime = 1;
    rate.u32SrcFrameRateNum = rate.fr32DstFrameRateNum = fps;
    rate.u32SrcFrameRateDen = rate.fr32DstFrameRateDen = 1;
    check(RK_MPI_VENC_CreateChn(0, &encoder), "Create VENC encoder");
    venc = true;
    VENC_RECV_PIC_PARAM_S receive{};
    receive.s32RecvPicNum = -1;
    check(RK_MPI_VENC_StartRecvFrame(0, &receive), "Start VENC");
    receiving = true;
    source = {RK_ID_VI, 0, 0};
    destination = {RK_ID_VENC, 0, 0};
    check(RK_MPI_SYS_Bind(&source, &destination), "Bind VI to VENC");
    bound = true;
    rate_dirty = true;
    orientation_dirty = true;
    rtp = std::make_unique<openhd::RTPHelper>(h265);
    rtp->set_out_cb([this](auto fragments) {
      if (!owner.m_output_cb || fragments.empty()) return;
      const auto& settings = owner.m_camera_holder->get_settings();
      owner.m_output_cb(owner.m_camera_holder->get_camera().index,
          openhd::FragmentedVideoFrame{std::move(fragments), capture_time,
              settings.enable_ultra_secure_encryption, nullptr, false, keyframe});
    });
    const auto& camera = owner.m_camera_holder->get_camera();
    openhd::LinkActionHandler::CamInfo info{true, static_cast<uint8_t>(camera.index),
        static_cast<uint8_t>(camera.camera_type), CameraStream::CAM_STATUS_RESTARTING,
        static_cast<uint8_t>(s.air_recording), static_cast<uint8_t>(video_codec_to_int(f.videoCodec)),
        static_cast<uint16_t>(bitrate.load()), static_cast<uint16_t>(bitrate.load()),
        static_cast<uint8_t>(s.h26x_keyframe_interval), static_cast<uint16_t>(width),
        static_cast<uint16_t>(height), static_cast<uint16_t>(fps), 0, 0, 0,
        static_cast<uint8_t>(s.qp_max), static_cast<uint8_t>(s.qp_min)};
    auto& actions = openhd::LinkActionHandler::instance();
    actions.set_cam_info(camera.index, info);
    actions.set_cam_info_supports_variable_bitrate(camera.index, true);
    log->info("Native Pico camera {}: {}x{}@{}, {} VI -> VENC; GStreamer unused",
              sensor.sensor_info.sensor_name, width, height, fps, h265 ? "H265" : "H264");
  }

  void controls() {
    if (rate_dirty.exchange(false)) {
      VENC_CHN_ATTR_S attributes{};
      check(RK_MPI_VENC_GetChnAttr(0, &attributes), "Read encoder bitrate");
      (h265 ? attributes.stRcAttr.stH265Cbr : attributes.stRcAttr.stH264Cbr).u32BitRate = bitrate;
      check(RK_MPI_VENC_SetChnAttr(0, &attributes), "Set encoder bitrate");
      VENC_RC_PARAM_S parameters{};
      check(RK_MPI_VENC_GetRcParam(0, &parameters), "Read encoder QP");
      const auto minimum = std::clamp(qp_min.load(), 0, 48);
      const auto maximum = std::clamp(qp_max.load(), std::max(8, minimum), 51);
      if (h265) {
        parameters.stParamH265.u32MinQp = parameters.stParamH265.u32MinIQp = minimum;
        parameters.stParamH265.u32MaxQp = parameters.stParamH265.u32MaxIQp = maximum;
      } else {
        parameters.stParamH264.u32MinQp = parameters.stParamH264.u32MinIQp = minimum;
        parameters.stParamH264.u32MaxQp = parameters.stParamH264.u32MaxIQp = maximum;
      }
      check(RK_MPI_VENC_SetRcParam(0, &parameters), "Set encoder QP");
    }
    if (orientation_dirty.exchange(false)) {
      const int value = orientation;
      const int flip = value & 3;
      VENC_CHN_ATTR_S attributes{};
      check(RK_MPI_VENC_GetChnAttr(0, &attributes), "Read encoder orientation");
      attributes.stVencAttr.enMirror = static_cast<MIRROR_E>(flip);
      check(RK_MPI_VENC_SetChnAttr(0, &attributes), "Set encoder mirror");
      check(RK_MPI_VENC_SetChnRotation(0, static_cast<ROTATION_E>(value >> 2)), "Set encoder rotation");
    }
    if (force_keyframe.exchange(false)) check(RK_MPI_VENC_RequestIDR(0, RK_FALSE), "Request IDR");
  }

  void cleanup() {
    recorder.close();
    if (bound) RK_MPI_SYS_UnBind(&source, &destination);
    if (receiving) RK_MPI_VENC_StopRecvFrame(0);
    if (venc) RK_MPI_VENC_DestroyChn(0);
    if (vi_channel) RK_MPI_VI_DisableChn(0, 0);
    if (vi_dev) RK_MPI_VI_DisableDev(0);
    if (aiq_started) rk_aiq_uapi2_sysctl_stop(aiq, false);
    if (aiq) rk_aiq_uapi2_sysctl_deinit(aiq);
    if (system_started) RK_MPI_SYS_Exit();
    bound = receiving = venc = vi_channel = vi_dev = aiq_started = system_started = false;
    aiq = nullptr;
    rtp.reset();
    auto& actions = openhd::LinkActionHandler::instance();
    const int index = owner.m_camera_holder->get_camera().index;
    actions.set_cam_info_status(index, CameraStream::CAM_STATUS_RESTARTING);
    actions.set_cam_info_perf(index, 0, 0);
    actions.set_cam_info_recording_active(index, false);
  }

  void capture() {
    VENC_PACK_S pack{};
    VENC_STREAM_S stream{};
    stream.pstPack = &pack;
    stream.u32PackCount = 1;
    auto window = Clock::now();
    auto last_frame = window;
    uint64_t bytes = 0, frames = 0;
    while (running) {
      controls();
      const auto result = RK_MPI_VENC_GetStream(0, &stream, 200);
      if (result != RK_SUCCESS) {
        if (Clock::now() - last_frame > std::chrono::seconds(5))
          throw std::runtime_error("Native Pico VENC produced no camera frames for 5 seconds");
        continue;
      }
      // Own encoded data before returning the DMA buffer to VENC.
      capture_time = openhd::mpp::capture_time_from_us(pack.u64PTS, Clock::now());
      std::vector<uint8_t> encoded;
      try {
        check(RK_MPI_SYS_MmzFlushCache(pack.pMbBlk, RK_TRUE), "Synchronize encoded DMA output");
        const auto* data = static_cast<const uint8_t*>(RK_MPI_MB_Handle2VirAddr(pack.pMbBlk));
        if (!data || pack.u32Len == 0) throw std::runtime_error("Empty VENC packet");
        // Rockit's packet MB handle resolves to this packet's valid address.
        // u32Offset describes its position in the encoder ring; adding it a
        // second time skips encoded bytes and reads unrelated ring contents.
        encoded.assign(data, data + pack.u32Len);
      } catch (...) {
        RK_MPI_VENC_ReleaseStream(0, &stream);
        throw;
      }
      check(RK_MPI_VENC_ReleaseStream(0, &stream), "Release encoded frame");
      last_frame = Clock::now();
      keyframe = openhd::mpp::annex_b_keyframe(encoded.data(), encoded.size(), h265);
      bytes += encoded.size();
      ++frames;
      rtp->feed_multiple_nalu(encoded.data(), encoded.size());
      const int index = owner.m_camera_holder->get_camera().index;
      auto& actions = openhd::LinkActionHandler::instance();
      actions.set_cam_info_status(index, CameraStream::CAM_STATUS_STREAMING);
      const auto mode = record_mode.load();
      const bool record = mode == AIR_RECORDING_ON || (mode == AIR_RECORDING_AUTO_ARM_DISARM && armed);
      if (record && !recorder.is_open()) {
        const auto path = openhd::video::create_unused_recording_filename(".mkv");
        if (!recorder.open(path, h265, width, height, fps))
          log->warn("Cannot open Pico recording {}", path);
        recording_start = capture_time;
        force_keyframe = true;
      }
      if (!record) recorder.close();
      if (recorder.is_open()) {
        const auto pts_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            capture_time - recording_start).count();
        recorder.feed_annex_b_access_unit(encoded.data(), encoded.size(), std::max<int64_t>(0, pts_ms));
      }
      actions.set_cam_info_recording_active(index, recorder.is_open());
      const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(last_frame - window).count();
      if (elapsed >= 1000) {
        actions.set_cam_info_perf(index, bytes * 8000 / elapsed, frames * 1000 / elapsed);
        bytes = frames = 0;
        window = last_frame;
      }
    }
  }

  void run() {
    while (running) {
      try {
        setup();
        capture();
      } catch (const std::exception& error) {
        log->error("Native Pico camera: {}", error.what());
      }
      cleanup();
      for (int i = 0; running && i < 20; ++i)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  RockchipMppStream& owner;
  std::shared_ptr<spdlog::logger> log = openhd::log::create_or_get("pico_mpi");
  std::thread thread;
  std::atomic<bool> running{false}, armed{false}, rate_dirty{true}, orientation_dirty{true}, force_keyframe{false};
  std::atomic<int> bitrate{8000}, qp_min{5}, qp_max{51}, orientation{0}, record_mode{0};
  int width = 0, height = 0, fps = 0, sensor_width = 0, sensor_height = 0;
  bool h265 = false, system_started = false, aiq_started = false, vi_dev = false;
  bool vi_channel = false, venc = false, receiving = false, bound = false;
  rk_aiq_sys_ctx_t* aiq = nullptr;
  MPP_CHN_S source{}, destination{};
  std::unique_ptr<openhd::RTPHelper> rtp;
  MatroskaRecorder recorder;
  Clock::time_point recording_start = Clock::now();
  Clock::time_point capture_time = Clock::now();
  bool keyframe = false;
};

RockchipMppStream::RockchipMppStream(std::shared_ptr<CameraHolder> holder, openhd::ON_ENCODE_FRAME_CB output)
    : CameraStream(std::move(holder), std::move(output)), m_impl(std::make_unique<Impl>(*this)) {}
RockchipMppStream::~RockchipMppStream() { terminate_looping(); }
void RockchipMppStream::start_looping() {
  if (!m_impl->running.exchange(true)) m_impl->thread = std::thread([this] { m_impl->run(); });
}
void RockchipMppStream::terminate_looping() {
  m_impl->running = false;
  if (m_impl->thread.joinable()) m_impl->thread.join();
}
void RockchipMppStream::request_keyframe() { m_impl->force_keyframe = true; }
void RockchipMppStream::set_qp_limits(int minimum, int maximum) {
  m_impl->qp_min = minimum; m_impl->qp_max = maximum; m_impl->rate_dirty = true;
}
void RockchipMppStream::set_orientation(int rotation, int flip) {
  m_impl->orientation = (rotation / 90 << 2) | flip; m_impl->orientation_dirty = true;
}
int RockchipMppStream::recording_setting() const { return m_impl->record_mode; }
void RockchipMppStream::handle_change_bitrate_request(openhd::LinkActionHandler::LinkBitrateInformation request) {
  m_impl->bitrate = m_camera_holder->clamp_video_bitrate_kbits(request.is_link_capacity_limit
      ? std::min(request.recommended_encoder_bitrate_kbits, m_camera_holder->get_settings().h26x_bitrate_kbits)
      : request.recommended_encoder_bitrate_kbits);
  m_impl->rate_dirty = true;
  openhd::LinkActionHandler::instance().set_cam_info_bitrate(m_camera_holder->get_camera().index, m_impl->bitrate);
}
void RockchipMppStream::handle_update_arming_state(bool armed) { m_impl->armed = armed; }
