#include "nxp_v4l2_stream.h"

#include <fcntl.h>
#include <linux/media-bus-format.h>
#include <linux/v4l2-subdev.h>
#include <linux/videodev2.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <deque>
#include <fstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "openhd_rtp.h"
#include "openhd_spdlog.h"

namespace {

constexpr uint32_t kBufferCount = 4;
constexpr uint32_t kEncodedBufferCount = 6;

int retry_ioctl(int fd, unsigned long request, void* argument) {
  int result;
  do {
    result = ioctl(fd, request, argument);
  } while (result < 0 && errno == EINTR);
  return result;
}

bool file_exists(const std::string& path) {
  return access(path.c_str(), F_OK) == 0;
}

bool set_subdev_format(const std::string& node, uint32_t pad, uint32_t width,
                       uint32_t height) {
  const int fd = open(node.c_str(), O_RDWR | O_CLOEXEC);
  if (fd < 0) return false;
  v4l2_subdev_format format{};
  format.which = V4L2_SUBDEV_FORMAT_ACTIVE;
  format.pad = pad;
  format.format.width = width;
  format.format.height = height;
  format.format.code = MEDIA_BUS_FMT_UYVY8_2X8;
  format.format.field = V4L2_FIELD_NONE;
  const bool ok = retry_ioctl(fd, VIDIOC_SUBDEV_S_FMT, &format) == 0;
  close(fd);
  return ok;
}

bool set_control(int fd, uint32_t id, int32_t value) {
  v4l2_control control{};
  control.id = id;
  control.value = value;
  return retry_ioctl(fd, VIDIOC_S_CTRL, &control) == 0;
}

bool get_control(int fd, uint32_t id, int32_t& value) {
  v4l2_control control{};
  control.id = id;
  if (retry_ioctl(fd, VIDIOC_G_CTRL, &control) < 0) return false;
  value = control.value;
  return true;
}

bool set_framerate(int fd, v4l2_buf_type type, uint32_t fps) {
  v4l2_streamparm parameters{};
  parameters.type = type;
  auto& time_per_frame =
      type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE
          ? parameters.parm.output.timeperframe
          : parameters.parm.capture.timeperframe;
  time_per_frame.numerator = 1;
  time_per_frame.denominator = std::max<uint32_t>(1, fps);
  return retry_ioctl(fd, VIDIOC_S_PARM, &parameters) == 0;
}

struct MappedPlane {
  void* data = MAP_FAILED;
  size_t length = 0;
};

struct MappedBuffer {
  std::vector<MappedPlane> planes;
};

void unmap_buffers(std::vector<MappedBuffer>& buffers) {
  for (auto& buffer : buffers) {
    for (auto& plane : buffer.planes) {
      if (plane.data != MAP_FAILED && plane.data != nullptr)
        munmap(plane.data, plane.length);
      plane.data = MAP_FAILED;
      plane.length = 0;
    }
  }
  buffers.clear();
}

bool allocate_mmap_buffers(int fd, v4l2_buf_type type, uint32_t plane_count,
                           uint32_t requested_count, bool queue_now,
                           std::vector<MappedBuffer>& buffers) {
  v4l2_requestbuffers request{};
  request.count = requested_count;
  request.type = type;
  request.memory = V4L2_MEMORY_MMAP;
  if (retry_ioctl(fd, VIDIOC_REQBUFS, &request) < 0 || request.count < 2)
    return false;

  buffers.resize(request.count);
  for (uint32_t index = 0; index < request.count; ++index) {
    v4l2_buffer buffer{};
    v4l2_plane planes[VIDEO_MAX_PLANES]{};
    buffer.type = type;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.index = index;
    buffer.m.planes = planes;
    buffer.length = plane_count;
    if (retry_ioctl(fd, VIDIOC_QUERYBUF, &buffer) < 0) return false;
    buffers[index].planes.resize(buffer.length);
    for (uint32_t plane_index = 0; plane_index < buffer.length;
         ++plane_index) {
      auto& mapped = buffers[index].planes[plane_index];
      mapped.length = planes[plane_index].length;
      mapped.data = mmap(nullptr, mapped.length, PROT_READ | PROT_WRITE,
                         MAP_SHARED, fd, planes[plane_index].m.mem_offset);
      if (mapped.data == MAP_FAILED) return false;
    }
    if (queue_now && retry_ioctl(fd, VIDIOC_QBUF, &buffer) < 0) return false;
  }
  return true;
}

}  // namespace

class NxpV4l2Stream::Impl {
 public:
  Impl(NxpV4l2Stream& owner, std::shared_ptr<spdlog::logger> log)
      : owner(owner), log(std::move(log)) {
    const auto& settings = owner.m_camera_holder->get_settings();
    bitrate_kbits = settings.h26x_bitrate_kbits;
    qp_min = settings.qp_min;
    qp_max = settings.qp_max;
  }

  void setup_rtp(bool use_h265) {
    rtp = std::make_shared<openhd::RTPHelper>(use_h265);
    rtp->set_out_cb([this](auto fragments) {
      if (!this->owner.m_output_cb || fragments.empty()) return;
      const auto& settings = this->owner.m_camera_holder->get_settings();
      openhd::FragmentedVideoFrame frame{
          std::move(fragments), std::chrono::steady_clock::now(),
          settings.enable_ultra_secure_encryption, nullptr, false,
          current_access_unit_is_keyframe.load()};
      this->owner.m_output_cb(
          this->owner.m_camera_holder->get_camera().index, frame);
    });
  }

  ~Impl() { stop(); }

  bool configure_orqa_media_graph() {
    const auto& camera = owner.m_camera_holder->get_camera();
    if (!camera.requires_orqa_pipeline()) return true;

    // The EVK device tree exposes sensor0/CSI0/ISI0 as video2 and
    // sensor1/CSI1/ISI1 as video3. Keep the two paths deterministic so a
    // secondary camera can use the other receiver.
    const bool second_path = camera.index > 0;
    capture_device = second_path ? "/dev/video3" : "/dev/video2";
    const std::string sensor =
        second_path ? "/dev/v4l-subdev2" : "/dev/v4l-subdev3";
    const std::string csi =
        second_path ? "/dev/v4l-subdev1" : "/dev/v4l-subdev0";
    if (!file_exists(capture_device) || !file_exists(sensor) ||
        !file_exists(csi)) {
      log->error("ORQA media nodes are incomplete for CSI path {}",
                 second_path ? 1 : 0);
      return false;
    }
    if (!set_subdev_format(sensor, 0, width, height) ||
        !set_subdev_format(csi, 0, width, height) ||
        !set_subdev_format(csi, 4, width, height)) {
      log->error("Cannot configure ORQA CSI{} media graph to {}x{} UYVY",
                 second_path ? 1 : 0, width, height);
      return false;
    }
    log->info("Native ORQA capture uses CSI{} {} at {}x{}",
              second_path ? 1 : 0, capture_device, width, height);
    return true;
  }

  bool setup_capture() {
    const auto& camera = owner.m_camera_holder->get_camera();
    if (camera.requires_orqa_pipeline()) {
      if (!configure_orqa_media_graph()) return false;
    } else {
      // Preserve the node used by the former NXP GStreamer pipeline. A second
      // NXP camera naturally uses the adjacent ISI capture node.
      capture_device = camera.index > 0 ? "/dev/video2" : "/dev/video3";
    }

    capture_fd =
        open(capture_device.c_str(), O_RDWR | O_NONBLOCK | O_CLOEXEC);
    if (capture_fd < 0) {
      log->error("Cannot open NXP capture node {}: {}", capture_device,
                 std::strerror(errno));
      return false;
    }

    v4l2_format format{};
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    format.fmt.pix_mp.width = width;
    format.fmt.pix_mp.height = height;
    format.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV12M;
    format.fmt.pix_mp.field = V4L2_FIELD_NONE;
    format.fmt.pix_mp.num_planes = 2;
    if (retry_ioctl(capture_fd, VIDIOC_S_FMT, &format) < 0 ||
        format.fmt.pix_mp.pixelformat != V4L2_PIX_FMT_NV12M ||
        format.fmt.pix_mp.width != width ||
        format.fmt.pix_mp.height != height) {
      log->error("{} rejected NV12M {}x{} capture: {}", capture_device, width,
                 height, std::strerror(errno));
      return false;
    }
    capture_plane_count = format.fmt.pix_mp.num_planes;
    for (uint32_t i = 0; i < capture_plane_count; ++i) {
      capture_strides[i] =
          std::max<uint32_t>(format.fmt.pix_mp.plane_fmt[i].bytesperline,
                             width);
    }
    if (!allocate_mmap_buffers(capture_fd,
                               V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE,
                               capture_plane_count, kBufferCount, true,
                               capture_buffers)) {
      log->error("Cannot allocate NXP capture buffers: {}",
                 std::strerror(errno));
      return false;
    }
    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    if (retry_ioctl(capture_fd, VIDIOC_STREAMON, &type) < 0) {
      log->error("Cannot start {}: {}", capture_device, std::strerror(errno));
      return false;
    }
    capture_streaming = true;
    return true;
  }

  bool setup_encoder() {
    encoder_fd = open("/dev/video0", O_RDWR | O_NONBLOCK | O_CLOEXEC);
    if (encoder_fd < 0) {
      log->error("Cannot open NXP VPU encoder /dev/video0: {}",
                 std::strerror(errno));
      return false;
    }
    v4l2_capability capability{};
    if (retry_ioctl(encoder_fd, VIDIOC_QUERYCAP, &capability) < 0 ||
        !(capability.device_caps & V4L2_CAP_VIDEO_M2M_MPLANE)) {
      log->error("/dev/video0 is not a multiplanar V4L2 mem2mem encoder");
      return false;
    }

    v4l2_format raw{};
    raw.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    raw.fmt.pix_mp.width = width;
    raw.fmt.pix_mp.height = height;
    raw.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV12M;
    raw.fmt.pix_mp.field = V4L2_FIELD_NONE;
    raw.fmt.pix_mp.num_planes = 2;
    if (retry_ioctl(encoder_fd, VIDIOC_S_FMT, &raw) < 0 ||
        raw.fmt.pix_mp.pixelformat != V4L2_PIX_FMT_NV12M) {
      log->error("NXP VPU rejected NV12M input: {}", std::strerror(errno));
      return false;
    }
    encoder_output_plane_count = raw.fmt.pix_mp.num_planes;
    // The VSI rate controller needs the real input cadence. Its default is
    // 30 fps; feeding this camera's 60 fps without S_PARM approximately
    // doubles the encoded bitrate even though the bitrate control reads back
    // correctly.
    if (!set_framerate(encoder_fd, V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE, fps))
      log->warn("NXP VPU rejected requested {} fps encoder input rate", fps);
    for (uint32_t i = 0; i < encoder_output_plane_count; ++i)
      encoder_output_strides[i] =
          std::max<uint32_t>(raw.fmt.pix_mp.plane_fmt[i].bytesperline, width);

    v4l2_format encoded{};
    encoded.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    encoded.fmt.pix_mp.width = width;
    encoded.fmt.pix_mp.height = height;
    encoded.fmt.pix_mp.pixelformat = h265 ? V4L2_PIX_FMT_HEVC
                                          : V4L2_PIX_FMT_H264;
    encoded.fmt.pix_mp.field = V4L2_FIELD_NONE;
    encoded.fmt.pix_mp.num_planes = 1;
    if (retry_ioctl(encoder_fd, VIDIOC_S_FMT, &encoded) < 0 ||
        encoded.fmt.pix_mp.pixelformat !=
            (h265 ? V4L2_PIX_FMT_HEVC : V4L2_PIX_FMT_H264)) {
      log->error("NXP VPU rejected {} output: {}", h265 ? "H.265" : "H.264",
                 std::strerror(errno));
      return false;
    }
    encoder_capture_plane_count = encoded.fmt.pix_mp.num_planes;

    apply_encoder_controls(true);
    if (!allocate_mmap_buffers(encoder_fd,
                               V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE,
                               encoder_output_plane_count, kBufferCount, false,
                               encoder_output_buffers)) {
      log->error("Cannot allocate NXP VPU input buffers: {}",
                 std::strerror(errno));
      return false;
    }
    for (uint32_t index = 0; index < encoder_output_buffers.size(); ++index)
      free_encoder_output.push_back(index);
    if (!allocate_mmap_buffers(encoder_fd,
                               V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE,
                               encoder_capture_plane_count,
                               kEncodedBufferCount, true,
                               encoder_capture_buffers)) {
      log->error("Cannot allocate NXP VPU output buffers: {}",
                 std::strerror(errno));
      return false;
    }

    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    if (retry_ioctl(encoder_fd, VIDIOC_STREAMON, &type) < 0) return false;
    encoder_capture_streaming = true;
    type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    if (retry_ioctl(encoder_fd, VIDIOC_STREAMON, &type) < 0) return false;
    encoder_output_streaming = true;
    return true;
  }

  void apply_encoder_controls(bool initial) {
    if (encoder_fd < 0) return;
    const auto& settings = owner.m_camera_holder->get_settings();
    const int bitrate = std::max(10, bitrate_kbits.load()) * 1000;
    if (initial &&
        !set_control(encoder_fd, V4L2_CID_MPEG_VIDEO_BITRATE_MODE,
                     V4L2_MPEG_VIDEO_BITRATE_MODE_CBR))
      log->warn("NXP VPU rejected constant bitrate mode");
    if (!set_control(encoder_fd, V4L2_CID_MPEG_VIDEO_BITRATE, bitrate)) {
      log->warn("NXP VPU rejected bitrate {} bit/s", bitrate);
    } else {
      int32_t readback = 0;
      if (get_control(encoder_fd, V4L2_CID_MPEG_VIDEO_BITRATE, readback)) {
        log->info("NXP VPU {} bitrate {} kbit/s (readback {} kbit/s)",
                  initial ? "initial" : "live", bitrate / 1000,
                  readback / 1000);
      } else {
        log->info("NXP VPU {} bitrate {} kbit/s",
                  initial ? "initial" : "live", bitrate / 1000);
      }
    }
    if (!initial) return;

    set_control(encoder_fd, V4L2_CID_MPEG_VIDEO_GOP_SIZE,
                std::max(1, settings.h26x_keyframe_interval));
    set_control(encoder_fd, V4L2_CID_MPEG_VIDEO_REPEAT_SEQ_HEADER, 1);
    if (h265) {
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_PROFILE
      set_control(encoder_fd, V4L2_CID_MPEG_VIDEO_HEVC_PROFILE,
                  V4L2_MPEG_VIDEO_HEVC_PROFILE_MAIN);
      set_control(encoder_fd, V4L2_CID_MPEG_VIDEO_HEVC_MIN_QP, qp_min.load());
      set_control(encoder_fd, V4L2_CID_MPEG_VIDEO_HEVC_MAX_QP, qp_max.load());
#endif
    } else {
      set_control(encoder_fd, V4L2_CID_MPEG_VIDEO_H264_PROFILE,
                  V4L2_MPEG_VIDEO_H264_PROFILE_CONSTRAINED_BASELINE);
      set_control(encoder_fd, V4L2_CID_MPEG_VIDEO_H264_MIN_QP, qp_min.load());
      set_control(encoder_fd, V4L2_CID_MPEG_VIDEO_H264_MAX_QP, qp_max.load());
    }
    set_control(encoder_fd, V4L2_CID_ROTATE,
                settings.camera_rotation_degree);
  }

  void apply_dynamic_controls() {
    if (rate_dirty.exchange(false)) apply_encoder_controls(false);
    if (qp_dirty.exchange(false)) {
      if (h265) {
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_MIN_QP
        set_control(encoder_fd, V4L2_CID_MPEG_VIDEO_HEVC_MIN_QP,
                    qp_min.load());
        set_control(encoder_fd, V4L2_CID_MPEG_VIDEO_HEVC_MAX_QP,
                    qp_max.load());
#endif
      } else {
        set_control(encoder_fd, V4L2_CID_MPEG_VIDEO_H264_MIN_QP,
                    qp_min.load());
        set_control(encoder_fd, V4L2_CID_MPEG_VIDEO_H264_MAX_QP,
                    qp_max.load());
      }
    }
    if (force_keyframe.exchange(false))
      set_control(encoder_fd, V4L2_CID_MPEG_VIDEO_FORCE_KEY_FRAME, 1);
  }

  bool copy_capture_to_encoder(const v4l2_buffer& capture,
                               const v4l2_plane* capture_planes) {
    if (free_encoder_output.empty() ||
        capture.index >= capture_buffers.size())
      return true;  // Drop rather than adding latency.
    const uint32_t index = free_encoder_output.front();
    free_encoder_output.pop_front();
    auto& destination = encoder_output_buffers[index];
    const auto& source = capture_buffers[capture.index];

    v4l2_buffer output{};
    v4l2_plane planes[VIDEO_MAX_PLANES]{};
    output.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    output.memory = V4L2_MEMORY_MMAP;
    output.index = index;
    output.m.planes = planes;
    output.length = encoder_output_plane_count;
    output.timestamp = capture.timestamp;
    output.flags = V4L2_BUF_FLAG_TIMESTAMP_COPY;
    for (uint32_t plane = 0;
         plane < encoder_output_plane_count && plane < source.planes.size() &&
         plane < destination.planes.size();
         ++plane) {
      const uint32_t rows = plane == 0 ? height : height / 2;
      const uint32_t source_stride = capture_strides[plane];
      const uint32_t destination_stride = encoder_output_strides[plane];
      const size_t row_bytes = std::min<uint32_t>(width, destination_stride);
      const auto* src = static_cast<const uint8_t*>(source.planes[plane].data) +
                        capture_planes[plane].data_offset;
      auto* dst = static_cast<uint8_t*>(destination.planes[plane].data);
      for (uint32_t row = 0; row < rows; ++row)
        std::memcpy(dst + static_cast<size_t>(row) * destination_stride,
                    src + static_cast<size_t>(row) * source_stride, row_bytes);
      planes[plane].bytesused = destination_stride * rows;
      planes[plane].length = destination.planes[plane].length;
    }
    if (retry_ioctl(encoder_fd, VIDIOC_QBUF, &output) < 0) {
      free_encoder_output.push_front(index);
      log->error("Cannot queue NXP VPU input: {}", std::strerror(errno));
      return false;
    }
    return true;
  }

  bool handle_capture_frame() {
    v4l2_buffer buffer{};
    v4l2_plane planes[VIDEO_MAX_PLANES]{};
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.m.planes = planes;
    buffer.length = capture_plane_count;
    if (retry_ioctl(capture_fd, VIDIOC_DQBUF, &buffer) < 0)
      return errno == EAGAIN;
    const bool ok = copy_capture_to_encoder(buffer, planes);
    if (retry_ioctl(capture_fd, VIDIOC_QBUF, &buffer) < 0) {
      log->error("Cannot requeue NXP camera buffer: {}", std::strerror(errno));
      return false;
    }
    return ok;
  }

  void reclaim_encoder_inputs() {
    while (true) {
      v4l2_buffer buffer{};
      v4l2_plane planes[VIDEO_MAX_PLANES]{};
      buffer.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
      buffer.memory = V4L2_MEMORY_MMAP;
      buffer.m.planes = planes;
      buffer.length = encoder_output_plane_count;
      if (retry_ioctl(encoder_fd, VIDIOC_DQBUF, &buffer) < 0) {
        if (errno != EAGAIN)
          log->warn("Cannot reclaim NXP VPU input: {}", std::strerror(errno));
        return;
      }
      free_encoder_output.push_back(buffer.index);
    }
  }

  void drain_encoded_access_units() {
    while (true) {
      v4l2_buffer buffer{};
      v4l2_plane planes[VIDEO_MAX_PLANES]{};
      buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
      buffer.memory = V4L2_MEMORY_MMAP;
      buffer.m.planes = planes;
      buffer.length = encoder_capture_plane_count;
      if (retry_ioctl(encoder_fd, VIDIOC_DQBUF, &buffer) < 0) {
        if (errno != EAGAIN)
          log->warn("Cannot dequeue NXP VPU output: {}", std::strerror(errno));
        return;
      }
      if (buffer.index < encoder_capture_buffers.size() &&
          planes[0].bytesused > 0) {
        const auto& mapped = encoder_capture_buffers[buffer.index].planes[0];
        const auto* data = static_cast<const uint8_t*>(mapped.data) +
                           planes[0].data_offset;
        const size_t available =
            planes[0].data_offset < mapped.length
                ? mapped.length - planes[0].data_offset
                : 0;
        const size_t size = std::min<size_t>(planes[0].bytesused, available);
        if (size > 0) {
          current_access_unit_is_keyframe =
              (buffer.flags & V4L2_BUF_FLAG_KEYFRAME) != 0;
          rtp->feed_multiple_nalu(data, static_cast<int>(size));
          perf_bytes += size;
          ++perf_frames;
          current_access_unit_is_keyframe = false;
        }
      }
      if (retry_ioctl(encoder_fd, VIDIOC_QBUF, &buffer) < 0) {
        log->error("Cannot requeue NXP VPU output: {}", std::strerror(errno));
        return;
      }
    }
  }

  void publish_cam_info() {
    const auto& settings = owner.m_camera_holder->get_settings();
    const auto& format = settings.streamed_video_format;
    const int camera_index = owner.m_camera_holder->get_camera().index;
    openhd::LinkActionHandler::CamInfo cam_info{
        true,
        static_cast<uint8_t>(camera_index),
        static_cast<uint8_t>(owner.m_camera_holder->get_camera().camera_type),
        CameraStream::CAM_STATUS_RESTARTING,
        static_cast<uint8_t>(settings.air_recording),
        static_cast<uint8_t>(video_codec_to_int(format.videoCodec)),
        static_cast<uint16_t>(settings.h26x_bitrate_kbits),
        static_cast<uint16_t>(settings.h26x_bitrate_kbits),
        static_cast<uint8_t>(settings.h26x_keyframe_interval),
        static_cast<uint16_t>(width),
        static_cast<uint16_t>(height),
        static_cast<uint16_t>(fps),
        0,
        0,
        0,
        static_cast<uint8_t>(settings.qp_max),
        static_cast<uint8_t>(settings.qp_min)};
    openhd::LinkActionHandler::instance().set_cam_info(camera_index, cam_info);
    openhd::LinkActionHandler::instance().set_cam_info_supports_variable_bitrate(
        camera_index, true);
  }

  void update_performance() {
    const auto now = std::chrono::steady_clock::now();
    const auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                                now - perf_started)
                                .count();
    if (elapsed_ms < 1000) return;
    const uint32_t bitrate = static_cast<uint32_t>(
        perf_bytes * 8ULL * 1000ULL /
        static_cast<uint64_t>(std::max<int64_t>(1, elapsed_ms)));
    const uint16_t actual_fps = static_cast<uint16_t>(
        perf_frames * 1000ULL /
        static_cast<uint64_t>(std::max<int64_t>(1, elapsed_ms)));
    openhd::LinkActionHandler::instance().set_cam_info_perf(
        owner.m_camera_holder->get_camera().index, bitrate, actual_fps);
    perf_bytes = 0;
    perf_frames = 0;
    perf_started = now;
  }

  void run() {
    while (running &&
           !owner.m_camera_holder->get_settings().enable_streaming)
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (!running) return;

    const auto& settings = owner.m_camera_holder->get_settings();
    width = std::max(136, settings.streamed_video_format.width);
    height = std::max(136, settings.streamed_video_format.height);
    fps = std::max(1, settings.streamed_video_format.framerate);
    h265 = settings.streamed_video_format.videoCodec == VideoCodec::H265;
    setup_rtp(h265);
    publish_cam_info();
    if (!setup_capture() || !setup_encoder()) {
      cleanup();
      running = false;
      return;
    }
    log->info("Native NXP V4L2 stream active: {} -> /dev/video0, {}x{}@{} {}",
              capture_device, width, height, fps, h265 ? "H.265" : "H.264");
    openhd::LinkActionHandler::instance().set_cam_info_status(
        owner.m_camera_holder->get_camera().index,
        CameraStream::CAM_STATUS_STREAMING);

    while (running) {
      apply_dynamic_controls();
      pollfd descriptors[2]{{capture_fd, POLLIN, 0},
                            {encoder_fd, POLLIN | POLLOUT, 0}};
      const int result = poll(descriptors, 2, 100);
      if (result < 0 && errno != EINTR) {
        log->error("Native NXP video poll failed: {}", std::strerror(errno));
        break;
      }
      reclaim_encoder_inputs();
      drain_encoded_access_units();
      if (result > 0 && (descriptors[0].revents & POLLIN) &&
          !handle_capture_frame())
        break;
      reclaim_encoder_inputs();
      drain_encoded_access_units();
      update_performance();
    }
    cleanup();
  }

  void cleanup() {
    if (capture_streaming && capture_fd >= 0) {
      v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
      retry_ioctl(capture_fd, VIDIOC_STREAMOFF, &type);
    }
    if (encoder_output_streaming && encoder_fd >= 0) {
      v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
      retry_ioctl(encoder_fd, VIDIOC_STREAMOFF, &type);
    }
    if (encoder_capture_streaming && encoder_fd >= 0) {
      v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
      retry_ioctl(encoder_fd, VIDIOC_STREAMOFF, &type);
    }
    unmap_buffers(capture_buffers);
    unmap_buffers(encoder_output_buffers);
    unmap_buffers(encoder_capture_buffers);
    if (capture_fd >= 0) close(capture_fd);
    if (encoder_fd >= 0) close(encoder_fd);
    capture_fd = -1;
    encoder_fd = -1;
    capture_streaming = false;
    encoder_output_streaming = false;
    encoder_capture_streaming = false;
    free_encoder_output.clear();
  }

  void stop() {
    running = false;
    if (thread.joinable()) thread.join();
  }

  NxpV4l2Stream& owner;
  std::shared_ptr<spdlog::logger> log;
  std::shared_ptr<openhd::RTPHelper> rtp;
  std::thread thread;
  std::atomic_bool running{false};
  std::atomic_bool rate_dirty{false};
  std::atomic_bool qp_dirty{false};
  std::atomic_bool force_keyframe{false};
  std::atomic_bool current_access_unit_is_keyframe{false};
  std::atomic_int bitrate_kbits{8000};
  std::atomic_int qp_min{5};
  std::atomic_int qp_max{51};
  int capture_fd = -1;
  int encoder_fd = -1;
  bool capture_streaming = false;
  bool encoder_output_streaming = false;
  bool encoder_capture_streaming = false;
  bool h265 = false;
  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t fps = 0;
  uint32_t capture_plane_count = 0;
  uint32_t encoder_output_plane_count = 0;
  uint32_t encoder_capture_plane_count = 0;
  uint32_t capture_strides[VIDEO_MAX_PLANES]{};
  uint32_t encoder_output_strides[VIDEO_MAX_PLANES]{};
  std::string capture_device;
  std::vector<MappedBuffer> capture_buffers;
  std::vector<MappedBuffer> encoder_output_buffers;
  std::vector<MappedBuffer> encoder_capture_buffers;
  std::deque<uint32_t> free_encoder_output;
  uint64_t perf_bytes = 0;
  uint32_t perf_frames = 0;
  std::chrono::steady_clock::time_point perf_started =
      std::chrono::steady_clock::now();
};

NxpV4l2Stream::NxpV4l2Stream(
    std::shared_ptr<CameraHolder> camera_holder,
    openhd::ON_ENCODE_FRAME_CB out_cb)
    : CameraStream(std::move(camera_holder), std::move(out_cb)),
      m_impl(std::make_unique<Impl>(
          *this, openhd::log::create_or_get("nxp_v4l2_stream"))) {
  m_camera_holder->register_listener(
      [this]() { terminate_looping(); start_looping(); });
  m_camera_holder->register_video_bitrate_listener(
      [this](int bitrate) { handle_change_bitrate_request({bitrate}); });
  m_camera_holder->register_video_qp_listener([this](int minimum, int maximum) {
    m_impl->qp_min = minimum;
    m_impl->qp_max = maximum;
    m_impl->qp_dirty = true;
  });
  m_camera_holder->register_video_force_keyframe_listener(
      [this]() { m_impl->force_keyframe = true; });
}

NxpV4l2Stream::~NxpV4l2Stream() {
  m_camera_holder->register_video_bitrate_listener(nullptr);
  m_camera_holder->register_video_qp_listener(nullptr);
  m_camera_holder->register_video_force_keyframe_listener(nullptr);
  terminate_looping();
}

void NxpV4l2Stream::start_looping() {
  if (m_impl->running.exchange(true)) return;
  m_impl->thread = std::thread([this]() { m_impl->run(); });
}

void NxpV4l2Stream::terminate_looping() { m_impl->stop(); }

void NxpV4l2Stream::handle_change_bitrate_request(
    openhd::LinkActionHandler::LinkBitrateInformation lb) {
  m_impl->bitrate_kbits = m_camera_holder->clamp_video_bitrate_kbits(
      lb.recommended_encoder_bitrate_kbits);
  m_impl->rate_dirty = true;
  openhd::LinkActionHandler::instance().set_cam_info_bitrate(
      m_camera_holder->get_camera().index,
      static_cast<uint16_t>(m_impl->bitrate_kbits.load()));
}

void NxpV4l2Stream::handle_update_arming_state(bool /*armed*/) {}
