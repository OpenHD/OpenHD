#include "orqa_hantro_encoder.h"

#include <fcntl.h>
#include <linux/videodev2.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstring>
#include <limits>
#include <utility>
#include <vector>

extern "C" {
// NXP's C headers typedef bool as a 32-bit enum in C, but silently use the
// one-byte C++ bool in C++. The shipped C library uses the former layout.
// Match that ABI inside these headers only (not in our C++ implementation).
#define bool uint32_t
#include <ewl.h>
#include <hevcencapi.h>
#undef bool
}
static_assert(offsetof(VCEncConfig, lookaheadDepth) == 136,
              "Unsupported Hantro C API configuration layout");

namespace {
int xioctl(int fd, unsigned long request, void* argument) {
  int result;
  do {
    result = ioctl(fd, request, argument);
  } while (result < 0 && errno == EINTR);
  return result;
}
int64_t monotonic_us() {
  timespec ts{};
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * 1000000LL + ts.tv_nsec / 1000;
}
constexpr unsigned kCaptureBuffers = 3;
constexpr unsigned kBitstreamBytes = 4 * 1024 * 1024;
}  // namespace

class OrqaHantroEncoder::Impl {
 public:
  Impl(Config config, Output output, Log log)
      : config(std::move(config)),
        output(std::move(output)),
        log(std::move(log)) {}
  ~Impl() { close(); }

  bool fail(const std::string& operation, int error) {
    if (log) log(operation + ": " + std::to_string(error));
    return false;
  }
  bool api(VCEncRet result, const char* operation) {
    return result == VCENC_OK || fail(operation, result);
  }

  bool open_capture() {
    fd = ::open(config.device.c_str(), O_RDWR | O_NONBLOCK | O_CLOEXEC);
    if (fd < 0) return fail("open capture " + config.device, errno);
    v4l2_capability caps{};
    if (xioctl(fd, VIDIOC_QUERYCAP, &caps) < 0)
      return fail("VIDIOC_QUERYCAP", errno);
    const auto capabilities = (caps.capabilities & V4L2_CAP_DEVICE_CAPS)
                                  ? caps.device_caps
                                  : caps.capabilities;
    if (!(capabilities & V4L2_CAP_STREAMING))
      return fail("capture not streaming capable", 0);
    multiplanar = (capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE) != 0;
    if (!multiplanar && !(capabilities & V4L2_CAP_VIDEO_CAPTURE))
      return fail("device is not a capture node", 0);
    type = multiplanar ? V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE
                       : V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2_format format{};
    format.type = type;
    if (multiplanar) {
      format.fmt.pix_mp.width = config.width;
      format.fmt.pix_mp.height = config.height;
      format.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_YUYV;
      format.fmt.pix_mp.field = V4L2_FIELD_NONE;
      format.fmt.pix_mp.num_planes = 1;
    } else {
      format.fmt.pix.width = config.width;
      format.fmt.pix.height = config.height;
      format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
      format.fmt.pix.field = V4L2_FIELD_NONE;
    }
    if (xioctl(fd, VIDIOC_S_FMT, &format) < 0)
      return fail("capture YUYV S_FMT", errno);
    unsigned width, height, fourcc;
    if (multiplanar) {
      width = format.fmt.pix_mp.width;
      height = format.fmt.pix_mp.height;
      fourcc = format.fmt.pix_mp.pixelformat;
      stride = format.fmt.pix_mp.plane_fmt[0].bytesperline;
      image_bytes = format.fmt.pix_mp.plane_fmt[0].sizeimage;
      if (format.fmt.pix_mp.num_planes != 1)
        return fail("YUYV must have one plane", 0);
    } else {
      width = format.fmt.pix.width;
      height = format.fmt.pix.height;
      fourcc = format.fmt.pix.pixelformat;
      stride = format.fmt.pix.bytesperline;
      image_bytes = format.fmt.pix.sizeimage;
    }
    if (log)
      log("Capture negotiated " + std::to_string(width) + "x" +
          std::to_string(height) + " fourcc=" + std::to_string(fourcc) +
          " stride=" + std::to_string(stride) +
          " size=" + std::to_string(image_bytes));
    if (width != config.width || height != config.height ||
        fourcc != V4L2_PIX_FMT_YUYV || stride < width * 2 || stride % 2 ||
        image_bytes < stride * height)
      return fail("capture rejected packed YUYV geometry/stride", 0);
    v4l2_streamparm parm{};
    parm.type = type;
    parm.parm.capture.timeperframe.numerator = 1;
    parm.parm.capture.timeperframe.denominator = config.fps;
    // Fixed-rate CSI bridges may not implement S_PARM.
    if (xioctl(fd, VIDIOC_S_PARM, &parm) < 0 && log)
      log("Capture uses camera's fixed cadence (S_PARM unavailable)");
    return true;
  }

  bool open_encoder() {
    VCEncConfig cfg{};
    cfg.streamType = VCENC_BYTE_STREAM;
    cfg.codecFormat =
        config.h265 ? VCENC_VIDEO_CODEC_HEVC : VCENC_VIDEO_CODEC_H264;
    cfg.profile =
        config.h265 ? VCENC_HEVC_MAIN_PROFILE : VCENC_H264_BASE_PROFILE;
    // Advertising AVC 5.1 unnecessarily rejects these streams on embedded
    // decoders. 1080p60 and 960x720p120 both fit level 4.2's macroblock limits.
    const uint64_t macroblocks =
        uint64_t((config.width + 15) / 16) * ((config.height + 15) / 16);
    cfg.level = config.h265 ? VCENC_HEVC_LEVEL_5_1
                           : (macroblocks <= 8704 &&
                              macroblocks * config.fps <= 522240 &&
                              config.bitrate_kbits <= 50000
                                  ? VCENC_H264_LEVEL_4_2
                                  : VCENC_H264_LEVEL_5_1);
    cfg.width = config.width;
    cfg.height = config.height;
    if (config.rotation == 90 || config.rotation == 270)
      std::swap(cfg.width, cfg.height);
    cfg.frameRateNum = config.fps;
    cfg.frameRateDenom = 1;
    cfg.refFrameAmount = 1;
    cfg.strongIntraSmoothing = 1;
    cfg.bitDepthLuma = cfg.bitDepthChroma = 8;
    cfg.maxTLayers = 1;
    cfg.rdoLevel = 1;
    cfg.exp_of_input_alignment = 4;
    cfg.exp_of_ref_alignment = cfg.exp_of_ref_ch_alignment = 4;
    cfg.parallelCoreNum = 1;
    cfg.pass = 0;
    cfg.lookaheadDepth = 0;
    cfg.log2MaxPicOrderCntLsb = 16;
    cfg.log2MaxFrameNum = 12;
    cfg.extSramLumHeightBwd = cfg.extSramLumHeightFwd = 16;
    cfg.extSramChrHeightBwd = cfg.extSramChrHeightFwd = 8;
    cfg.codedChromaIdc = VCENC_CHROMA_IDC_420;
    const auto initialized = VCEncInit(&cfg, &encoder);
    if (initialized != VCENC_OK) {
      encoder = nullptr;  // Failed init may leave a non-owning partial handle.
      return fail("VCEncInit", initialized);
    }
    ewl = VCEncGetEwl(encoder);
    if (!ewl) return fail("VCEncGetEwl", 0);

    VCEncCodingCtrl coding{};
    if (!api(VCEncGetCodingCtrl(encoder, &coding), "VCEncGetCodingCtrl"))
      return false;
    const unsigned rows =
        (cfg.height + (config.h265 ? 63 : 15)) / (config.h265 ? 64 : 16);
    coding.sliceSize =
        config.slices > 1 ? (rows + config.slices - 1) / config.slices : 0;
    coding.seiMessages = 0;
    coding.enableCabac = 0;
    coding.inputLineBufEn = 0;  // Frame capture, no unproven line handshake.
    coding.streamMultiSegmentMode = 0;
    if (!api(VCEncSetCodingCtrl(encoder, &coding), "VCEncSetCodingCtrl"))
      return false;
    if (!set_rate(config.bitrate_kbits, config.qp_min, config.qp_max))
      return false;

    VCEncPreProcessingCfg preprocess{};
    if (!api(VCEncGetPreProcessing(encoder, &preprocess),
             "VCEncGetPreProcessing"))
      return false;
    preprocess.origWidth = stride / 2;
    preprocess.origHeight = config.height;
    preprocess.inputType = VCENC_YUV422_INTERLEAVED_YUYV;
    preprocess.rotation = config.rotation == 90    ? VCENC_ROTATE_90R
                          : config.rotation == 270 ? VCENC_ROTATE_90L
                          : config.rotation == 180 ? VCENC_ROTATE_180R
                                                   : VCENC_ROTATE_0;
    preprocess.xOffset = preprocess.yOffset = 0;
    if (!api(VCEncSetPreProcessing(encoder, &preprocess),
             "VCEncSetPreProcessing"))
      return false;

    inputs.resize(kCaptureBuffers);
    for (auto& memory : inputs) {
      if (EWLMallocLinear(ewl, image_bytes, 64, &memory) != 0)
        return fail("allocate shared capture/encoder memory", errno);
    }
    if (EWLMallocLinear(ewl, kBitstreamBytes, 64, &bitstream) != 0)
      return fail("allocate bitstream memory", errno);
    in.pOutBuf[0] = bitstream.virtualAddress;
    in.busOutBuf[0] = bitstream.busAddress;
    in.outBufSize[0] = bitstream.size;
    // IPPP only: each P picture references the immediately preceding picture.
    pic.poc = 1;
    // The API takes sqrt(lambda factor), as in NXP's GOP parser.
    pic.QpFactor = std::sqrt(config.h265 ? 0.578 : 0.4);
    pic.codingType = VCENC_PREDICTED_FRAME;
    pic.numRefPics = 1;
    pic.refPics[0].ref_pic = -1;
    pic.refPics[0].used_by_cur = 1;
    in.gopConfig.pGopPicCfg = &pic;
    in.gopConfig.size = 1;
    in.gopConfig.delta_poc_to_next = 1;
    in.gopConfig.idr_interval = config.keyframe_interval;
    in.gopConfig.lastPic = std::numeric_limits<int>::max();
    in.gopConfig.outputRateNumer = in.gopConfig.inputRateNumer = config.fps;
    in.gopConfig.outputRateDenom = in.gopConfig.inputRateDenom = 1;
    in.gopConfig.gopLowdelay = 1;
    in.gopSize = 1;
    reset_current_picture();
    in.i8SpecialRpsIdx = in.i8SpecialRpsIdx_next = -1;
    in.vui_timing_info_enable = 1;
    for (auto& reference : in.long_term_ref_pic) reference = -1;
    VCEncOut out{};
    if (!api(VCEncStrmStart(encoder, &in, &out), "VCEncStrmStart"))
      return false;
    encoder_started = true;
    VCEncFindNextPic(encoder, &in, 1, in.gopConfig.gopCfgOffset, 1);
    if (!emit(out, false, 0)) return false;
    return true;
  }

  bool queue(unsigned index) {
    v4l2_buffer buffer{};
    v4l2_plane plane{};
    buffer.type = type;
    buffer.memory = memory_type;
    buffer.index = index;
    auto& memory = inputs[index];
    if (multiplanar) {
      buffer.length = 1;
      buffer.m.planes = &plane;
      plane.length = memory.size;
      if (memory_type == V4L2_MEMORY_USERPTR)
        plane.m.userptr =
            reinterpret_cast<unsigned long>(memory.virtualAddress);
      else
        plane.m.fd = memory.ion_fd;
    } else {
      buffer.length = memory.size;
      if (memory_type == V4L2_MEMORY_USERPTR)
        buffer.m.userptr =
            reinterpret_cast<unsigned long>(memory.virtualAddress);
      else
        buffer.m.fd = memory.ion_fd;
    }
    return xioctl(fd, VIDIOC_QBUF, &buffer) == 0 || fail("capture QBUF", errno);
  }

  bool start() {
    if (fd >= 0 || encoder) return fail("encoder already opened", 0);
    if (config.width < 136 || config.width > 1920 || config.height < 96 ||
        config.height > 1088 || config.width % 2 || config.height % 2 ||
        config.fps == 0 || config.fps > 240 || config.keyframe_interval == 0 ||
        config.qp_min > config.qp_max || config.qp_max > 51)
      return fail("invalid direct encoder configuration", 0);
    if (!open_capture() || !open_encoder()) return false;
    v4l2_requestbuffers request{};
    request.type = type;
    request.count = kCaptureBuffers;
    request.memory = memory_type = V4L2_MEMORY_USERPTR;
    if (xioctl(fd, VIDIOC_REQBUFS, &request) < 0) {
      request.count = kCaptureBuffers;
      request.memory = memory_type = V4L2_MEMORY_DMABUF;
      for (const auto& memory : inputs)
        if (memory.ion_fd < 0)
          return fail("EWL allocation has no DMA-BUF fd", 0);
      if (xioctl(fd, VIDIOC_REQBUFS, &request) < 0)
        return fail("capture REQBUFS", errno);
    }
    if (request.count < 2 || request.count > inputs.size())
      return fail("capture returned unsupported buffer count", request.count);
    buffer_count = request.count;
    for (unsigned i = 0; i < buffer_count; ++i)
      if (!queue(i)) return false;
    if (xioctl(fd, VIDIOC_STREAMON, &type) < 0)
      return fail("capture STREAMON", errno);
    capture_started = true;
    last_capture_us = monotonic_us();
    if (log)
      log("Direct Hantro active: " + config.device + " YUYV shared " +
          (memory_type == V4L2_MEMORY_USERPTR ? "USERPTR" : "DMA-BUF") +
          ", buffers=" + std::to_string(buffer_count) +
          ", raw copies=0, B frames=0, lookahead=0");
    return true;
  }

  bool program_quantizer(unsigned bitrate, int qp) {
    VCEncRateCtrl rate{};
    if (!api(VCEncGetRateCtrl(encoder, &rate), "VCEncGetRateCtrl")) return false;
    // Keep the requested bitrate fixed. The SDK's picture RC can get stuck at
    // QP 51; its qpHdr setter alone also does not reliably apply live QP.
    // Pin its I/P bounds to the selected quantizer and control picture quality
    // here, within the user's QP limits. There is no link/drop/loss input.
    rate.pictureRc = 0;
    rate.ctbRc = 0;
    rate.vbr = 0;
    rate.hrd = 0;
    rate.pictureSkip = 0;
    rate.crf = -1;
    rate.qpHdr = qp;
    rate.intraQpDelta = 0;
    rate.fixedIntraQp = 0;
    rate.qpMinI = rate.qpMinPB = qp;
    rate.qpMaxI = rate.qpMaxPB = qp;
    rate.bitPerSecond = bitrate * 1000;
    if (!api(VCEncSetRateCtrl(encoder, &rate), "VCEncSetRateCtrl")) return false;
    applied_qp = qp;
    return true;
  }

  bool set_rate(unsigned bitrate, unsigned minimum, unsigned maximum) {
    if (!encoder || minimum > maximum || maximum > 51 || bitrate < 10 || bitrate > 288000)
      return fail("invalid direct encoder rate", bitrate);
    // The link repeats its capacity recommendation. An unchanged request must
    // not reset quantizer tracking or reprogram the vendor API.
    if (encoder_started && bitrate == config.bitrate_kbits &&
        minimum == config.qp_min && maximum == config.qp_max) return true;
    if (!encoder_started) quantizer = config.h265 ? 32 : 35;
    else if (bitrate != config.bitrate_kbits)
      quantizer += std::clamp(6.0 * std::log2(double(config.bitrate_kbits) / bitrate), -6.0, 6.0);
    quantizer = std::clamp(quantizer, double(minimum), double(maximum));
    if (!program_quantizer(bitrate, std::lround(quantizer))) return false;
    config.bitrate_kbits = bitrate;
    config.qp_min = minimum;
    config.qp_max = maximum;
    rate_window_started_us = 0;
    rate_window_bytes = 0;
    if (log) log("Hantro fixed bitrate device=" + config.device + " target=" + std::to_string(bitrate) + " kbit/s");
    return true;
  }

  bool update_quantizer(unsigned bytes, int64_t now_us) {
    if (!rate_window_started_us) rate_window_started_us = now_us - 1000000 / config.fps;
    rate_window_bytes += bytes;
    // Include an entire IDR period, rather than reacting to its large I picture
    // independently. Only QP changes; the link's bitrate request stays intact.
    if (frames_since_idr < config.keyframe_interval) return true;
    const auto elapsed = now_us - rate_window_started_us;
    const double measured = double(rate_window_bytes) * 8000 / std::max<int64_t>(1, elapsed);
    rate_window_started_us = now_us;
    rate_window_bytes = 0;
    if (measured <= 0) return true;

    // Approximately six QP steps halve the byte rate. Apply half the inferred
    // correction per period, bounded to avoid sudden quality jumps. Clamping
    // the state as well as the programmed QP prevents integral windup.
    quantizer += std::clamp(3.0 * std::log2(measured / config.bitrate_kbits), -3.0, 3.0);
    quantizer = std::clamp(quantizer, double(config.qp_min), double(config.qp_max));
    const int qp = std::lround(quantizer);
    if (qp == applied_qp) return true;
    return program_quantizer(config.bitrate_kbits, qp);
  }
  bool emit(const VCEncOut& out, bool keyframe, int64_t capture_us) {
    if (out.streamSize > bitstream.size)
      return fail("encoder output exceeds allocation", out.streamSize);
    if (out.streamSize && output)
      output(reinterpret_cast<uint8_t*>(bitstream.virtualAddress),
             out.streamSize, keyframe, capture_us);
    return true;
  }

  void reset_current_picture() { in.gopCurrPicConfig = pic; }

  bool process(int timeout_ms) {
    if (!capture_started) return false;
    pollfd descriptor{fd, POLLIN, 0};
    const int polled = poll(&descriptor, 1, timeout_ms);
    if (polled < 0) return errno == EINTR || fail("capture poll", errno);
    if (polled == 0) {
      if (monotonic_us() - last_capture_us > 2000000)
        return fail("capture stalled for 2 seconds", 0);
      return true;
    }
    if (descriptor.revents & (POLLERR | POLLHUP | POLLNVAL))
      return fail("capture poll error", descriptor.revents);
    // Bound the drain to the buffer count: queue ownership stays with us until
    // the previous image has been encoded or deliberately dropped.
    int newest = -1;
    int64_t capture_us = 0;
    for (unsigned n = 0; n < buffer_count; ++n) {
      v4l2_buffer buffer{};
      v4l2_plane plane{};
      buffer.type = type;
      buffer.memory = memory_type;
      if (multiplanar) {
        buffer.m.planes = &plane;
        buffer.length = 1;
      }
      if (xioctl(fd, VIDIOC_DQBUF, &buffer) < 0) {
        if (errno == EAGAIN) break;
        return fail("capture DQBUF", errno);
      }
      if (buffer.index >= buffer_count)
        return fail("capture buffer index out of bounds", buffer.index);
      if (buffer.flags & V4L2_BUF_FLAG_ERROR) {
        ++statistics.dropped;
        if (!queue(buffer.index)) return false;
        continue;
      }
      const auto used = multiplanar ? plane.bytesused : buffer.bytesused;
      if (used < image_bytes || used > inputs[buffer.index].size ||
          (multiplanar && plane.data_offset != 0))
        return fail("capture returned incomplete/offset image", used);
      if (newest >= 0) {
        ++statistics.dropped;
        if (!queue(newest)) return false;
      }
      newest = buffer.index;
      capture_us =
          (buffer.flags & V4L2_BUF_FLAG_TIMESTAMP_MASK) ==
                  V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC
              ? buffer.timestamp.tv_sec * 1000000LL + buffer.timestamp.tv_usec
              : 0;
    }
    if (newest < 0) return true;
    last_capture_us = monotonic_us();
    in.busLuma = inputs[newest].busAddress;
    in.timeIncrement = statistics.frames == 0 ? 0 : 1;
    const bool idr =
        force_keyframe || frames_since_idr >= config.keyframe_interval;
    if (idr) {
      in.poc = 0;
      frames_since_idr = 0;
    }
    in.codingType = idr ? VCENC_INTRA_FRAME : VCENC_PREDICTED_FRAME;
    in.bIsIDR = idr;
    in.picture_cnt = statistics.frames;
    in.last_idr_picture_cnt = statistics.frames - frames_since_idr;
    reset_current_picture();
    in.gopCurrPicConfig.poc = in.poc;
    in.gopCurrPicConfig.codingType = in.codingType;
    in.gopCurrPicConfig.numRefPics = idr ? 0 : 1;
    VCEncOut out{};
    const auto started = monotonic_us();
    const auto result = VCEncStrmEncode(encoder, &in, &out, nullptr, nullptr);
    const auto finished = monotonic_us();
    // Synchronous single-core, single-pass API: hardware has finished reading
    // this input before it can be returned to the capture engine.
    if (result != VCENC_FRAME_READY)
      return fail("VCEncStrmEncode (expected FRAME_READY)", result);
    if (!queue(newest)) return false;
    force_keyframe = false;
    ++frames_since_idr;
    VCEncFindNextPic(encoder, &in, 1, in.gopConfig.gopCfgOffset, 0);
    ++statistics.frames;
#ifdef OPENHD_HANTRO_HAS_AVERAGE_QP
    if (log && statistics.frames % 600 == 0) {
      int average_qp = 0;
      if (VCEncGetAverageQp(encoder, &average_qp) == VCENC_OK)
        log("Hantro device=" + config.device +
            " frame=" + std::to_string(statistics.frames) +
            " average QP=" + std::to_string(average_qp));
    }
#endif
    statistics.bytes += out.streamSize;
    const uint64_t duration = finished - started;
    statistics.encode_us += duration;
    statistics.max_encode_us = std::max(statistics.max_encode_us, duration);
    if (capture_us > 0 && finished >= capture_us) {
      const uint64_t age = finished - capture_us;
      statistics.capture_to_output_us += age;
      statistics.max_capture_to_output_us =
          std::max(statistics.max_capture_to_output_us, age);
    }
    return emit(out, out.codingType == VCENC_INTRA_FRAME, capture_us) &&
           update_quantizer(out.streamSize, capture_us > 0 ? capture_us : last_capture_us);
  }

  void close() {
    // Release all capture references before freeing the shared allocations.
    if (capture_started) xioctl(fd, VIDIOC_STREAMOFF, &type);
    if (fd >= 0) ::close(fd);
    fd = -1;
    capture_started = false;
    if (encoder_started) {
      VCEncOut out{};
      VCEncStrmEnd(encoder, &in, &out);
    }
    if (ewl) {
      for (auto& memory : inputs)
        if (memory.virtualAddress) EWLFreeLinear(ewl, &memory);
      if (bitstream.virtualAddress) EWLFreeLinear(ewl, &bitstream);
    }
    inputs.clear();
    if (encoder) VCEncRelease(encoder);
    encoder = nullptr;
    ewl = nullptr;
  }

  Config config;
  Output output;
  Log log;
  Stats statistics;
  int fd = -1;
  bool multiplanar = false, capture_started = false, encoder_started = false;
  bool force_keyframe = true;
  unsigned buffer_count = 0, stride = 0, image_bytes = 0, frames_since_idr = 0;
  int64_t last_capture_us = 0;
  int applied_qp = 35;
  double quantizer = 35;
  int64_t rate_window_started_us = 0;
  uint64_t rate_window_bytes = 0;
  v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  v4l2_memory memory_type = V4L2_MEMORY_USERPTR;
  VCEncInst encoder = nullptr;
  const void* ewl = nullptr;
  std::vector<EWLLinearMem_t> inputs;
  EWLLinearMem_t bitstream{};
  VCEncIn in{};
  VCEncGopPicConfig pic{};
};

OrqaHantroEncoder::OrqaHantroEncoder(Config config, Output output, Log log)
    : impl(std::make_unique<Impl>(std::move(config), std::move(output),
                                  std::move(log))) {}
OrqaHantroEncoder::~OrqaHantroEncoder() = default;
bool OrqaHantroEncoder::start() { return impl->start(); }
bool OrqaHantroEncoder::process(int timeout_ms) {
  return impl->process(timeout_ms);
}
bool OrqaHantroEncoder::set_rate(unsigned bitrate, unsigned minimum,
                                 unsigned maximum) {
  return impl->set_rate(bitrate, minimum, maximum);
}
void OrqaHantroEncoder::request_keyframe() { impl->force_keyframe = true; }
const OrqaHantroEncoder::Stats& OrqaHantroEncoder::stats() const {
  return impl->statistics;
}
