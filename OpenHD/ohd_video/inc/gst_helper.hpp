//
// Created by consti10 on 15.05.22.
//

#ifndef OPENHD_OHDGSTHELPER_H
#define OPENHD_OHDGSTHELPER_H

#include <gst/gst.h>

#include <sstream>
#include <string>

#include "camera_settings.hpp"
#include "libcamera_iq_helper.h"
#include "openhd_bitrate.h"
#include "openhd_platform.h"
#include "openhd_spdlog.h"
#include "openhd_spdlog_include.h"
#include "openhd_util_filesystem.h"

// #define EXPERIMENTAL_USE_OPENH264_ENCODER

/**
 * Helper methods to create parts of gstreamer pipes.
 * Note: Unless a pipeline part is used at the end, all pipelines should end
 * with '! '. This way we avoid syntax errors.
 */
namespace OHDGstHelper {
/**
 * Check if we can find gstreamer at run time, throw a runtime error if not.
 */
static void initGstreamerOrThrow() {
  GError* error = nullptr;
  if (!gst_init_check(nullptr, nullptr, &error)) {
    openhd::log::get_default()->error("gst_init_check() failed: {}",
                                      error->message);
    g_error_free(error);
    throw std::runtime_error("GStreamer initialization failed");
  }
}

static std::string createCiscoH264SwEncoder(const CameraSettings& settings) {
  return fmt::format(
      "openh264enc name=swencoder complexity=low bitrate={} num-slices=4 "
      "slice-mode=1 rate-control=bitrate gop-size={} !",
      openhd::kbits_to_bits_per_second(settings.h26x_bitrate_kbits),
      // settings.h26x_num_slices,
      settings.h26x_keyframe_interval);
}

// SW encoding is slow, but should work on all platforms (at least for low
// resolutions/framerate(s) ) Note that not every sw encoder accepts every type
// of input format - I420 seems to work for all of them though.
static std::string createSwEncoder(const CameraSettings& settings) {
  std::stringstream ss;
  if (settings.streamed_video_format.videoCodec == VideoCodec::H264) {
#ifdef EXPERIMENTAL_USE_OPENH264_ENCODER
    ss << createCiscoH264SwEncoder(settings);
#else
    // Now this is a bit annoying - we cannot deal with frame(s) using sliced
    // encoding yet,so we have to disable it. But from that we get quite high
    // latency, due to how x264enc needs to parallelize encoding. By using
    // threads=2 we can reduce this issue a bit - and it probably is a good idea
    // anyways to do so, since on platforms like rpi we do not want to hog too
    // much of the CPU to not overload the system and on x86 2 threads / cores
    // are enough for sw encode of most resolutions anyways. NOTE: While not
    // exactly true, latency is ~ as many frame(s) as there are threads, aka 2
    // frames for 2 threads dct8x8=true
    std::string slices_str;
    if (settings.h26x_num_slices >= 2) {
      slices_str = fmt::format(" option-string=\"slices={}\" ",
                               settings.h26x_num_slices);
    }
    ss << fmt::format(
        "x264enc name=swencoder bitrate={} speed-preset=ultrafast  "
        "tune=zerolatency key-int-max={} sliced-threads=false threads=2"
        " intra-refresh={} qp_min=2 qp_step=10 {}! ",
        settings.h26x_bitrate_kbits, settings.h26x_keyframe_interval,
        settings.h26x_intra_refresh_type < 0 ? "false" : "true", slices_str);
#endif
  } else if (settings.streamed_video_format.videoCodec == VideoCodec::H265) {
    ss << fmt::format(
        "x265enc name=swencoder bitrate={} speed-preset=ultrafast "
        "tune=zerolatency key-int-max={} ! ",
        settings.h26x_bitrate_kbits, settings.h26x_keyframe_interval);
  }
  return ss.str();
}

static std::string gst_create_rtp_caps(const VideoCodec& videoCodec) {
  std::stringstream ss;
  if (videoCodec == VideoCodec::H264) {
    ss << "caps=\"application/x-rtp, media=(string)video, "
          "encoding-name=(string)H264, payload=(int)96\"";
  } else if (videoCodec == VideoCodec::H265) {
    ss << "caps=\"application/x-rtp, media=(string)video, "
          "encoding-name=(string)H265\"";
  }
  return ss.str();
}
// helper for common pipeline part(s)
// https://gstreamer.freedesktop.org/documentation/rtp/rtph264pay.html?gi-language=c
static std::string create_rtp_packetize_for_codec(const VideoCodec codec,
                                                  const uint32_t mtu) {
  if (codec == VideoCodec::H264)
    return fmt::format("rtph264pay mtu={} ! ", mtu);
  if (codec == VideoCodec::H265)
    return fmt::format("rtph265pay mtu={} ! ", mtu);
  assert(false);
  return "";
}

static std::string create_rtp_depacketize_for_codec(const VideoCodec& codec) {
  if (codec == VideoCodec::H264) return "rtph264depay ! ";
  if (codec == VideoCodec::H265) return "rtph265depay ! ";
  assert(false);
  return "";
}
static std::string create_parse_for_codec(const VideoCodec& codec) {
  // config-interval=-1 = makes 100% sure each keyframe has SPS and PPS
  if (codec == VideoCodec::H264) return "h264parse config-interval=-1 ! ";
  if (codec == VideoCodec::H265) return "h265parse config-interval=-1  ! ";
  assert(false);
  return "";
}

// a createXXXStream function always ends wth an encoded "h264,h265
// stream ! " aka after that, one can add a rtp encoder or similar. All these
// methods also start from zero - aka have a source like videotestsrc,
// nvarguscamerasr usw in the beginning and end with a OpenHD supported video
// codec
// ------------- crateXXXStream begin -------------
/**
 * Create a encoded dummy stream for the selected video format, that means a
 * stream that takes raw data coming from a videotestsrc and encodes it in
 * either h264 or h265
 */
static std::string createDummyStream(const CameraSettings& settings) {
  std::stringstream ss;
  ss << "videotestsrc name=videotestsrc ! ";
  // h265 cannot do NV12, but I420.
  // x264 can do both NV12 and I420
  // so we use I420 here since every SW encoder can do it.
  ss << fmt::format(
      "video/x-raw, format=I420,width={},height={},framerate={}/1 ! ",
      settings.streamed_video_format.width,
      settings.streamed_video_format.height,
      settings.streamed_video_format.framerate);
  // since the primary purpose here is testing, use sw encoder, which is always
  // guaranteed to work
  ss << createSwEncoder(settings);
  return ss.str();
}

/**
 * Create a encoded stream for rpicamsrc, which supports h264 encode in HW, and
 * h265 in SW (unusably slow)
 * @param camera_number use -1 to let rpicamsrc decide
 * See
 * https://gstreamer.freedesktop.org/documentation/rpicamsrc/index.html?gi-language=c#GstRpiCamSrcAWBMode
 * for more complicated params
 */
static std::string createRpicamsrcStream(
    const int camera_number, const CameraSettings& settings,
    const bool hdmi_to_csi_workaround_half_bitrate) {
  assert(settings.streamed_video_format.isValid());
  // assert(videoFormat.videoCodec == VideoCodec::H264);
  std::stringstream ss;
  // other than the other ones, rpicamsrc takes bit/s instead of kbit/s
  int bitrateBitsPerSecond =
      openhd::kbits_to_bits_per_second(settings.h26x_bitrate_kbits);
  if (hdmi_to_csi_workaround_half_bitrate) {
    openhd::log::get_default()->debug(
        "applying hack - reduce bitrate by 2 to get actual correct bitrate");
    bitrateBitsPerSecond = bitrateBitsPerSecond / 2;
  }
  if (camera_number == -1) {
    ss << fmt::format("rpicamsrc name=rpicamsrc bitrate={} preview=0 ",
                      bitrateBitsPerSecond);
  } else {
    ss << fmt::format(
        "rpicamsrc name=rpicamsrc camera-number={} bitrate={} preview=0 ",
        camera_number, bitrateBitsPerSecond);
  }
  // NOTE: These 2 params require the openhd rpicamsrc -
  if (true) {
    // qp-max=51 is the default in rpi v4l2 encoder wrapper
    ss << "qp-min=10 qp-max=51 ";
  }
  // keyframe-interval   : Interval (in frames) between I frames. -1 =
  // automatic, 0 = single-keyframe
  if (openhd::validate_rpi_keyframe_interval(settings.h26x_keyframe_interval)) {
    ss << "keyframe-interval=" << settings.h26x_keyframe_interval << " ";
  }
  if (openhd::validate_rpi_intra_refresh_type(
          settings.h26x_intra_refresh_type)) {
    ss << "intra-refresh-type=" << settings.h26x_intra_refresh_type << " ";
  }
  if (openhd::validate_camera_rotation(settings.camera_rotation_degree)) {
    ss << "rotation=" << settings.camera_rotation_degree << " ";
  }
  if (requires_hflip(settings)) {
    ss << "hflip=1 ";
  }
  if (requires_vflip(settings)) {
    ss << "vflip=1 ";
  }
  // Note: ROI (Region of interest) on the rpi does not tell the encoder to
  // allocate more bandwidth at a specific area, but rather zooms in on a
  // specific area (which is not really of use to use)
  ss << " ! ";
  if (settings.streamed_video_format.videoCodec == VideoCodec::H264) {
    if (settings.force_sw_encode) {
      openhd::log::get_default()->warn("Forced SW encode");
      ss << fmt::format("video/x-raw, width={}, height={}, framerate={}/1 ! ",
                        settings.streamed_video_format.width,
                        settings.streamed_video_format.height,
                        settings.streamed_video_format.framerate);
      ss << createSwEncoder(settings);
    } else {
      ss << fmt::format(
          "video/x-h264, profile=constrained-baseline, width={}, height={}, "
          "framerate={}/1, level=4.0 ! ",
          settings.streamed_video_format.width,
          settings.streamed_video_format.height,
          settings.streamed_video_format.framerate);
    }
  } else {
    openhd::log::get_default()->warn(
        "No h265 encoder on rpi, using SW encode (might result in frame "
        "drops/performance issues");
    ss << fmt::format("video/x-raw, width={}, height={}, framerate={}/1 ! ",
                      settings.streamed_video_format.width,
                      settings.streamed_video_format.height,
                      settings.streamed_video_format.framerate);
    ss << createSwEncoder(settings);
  }
  return ss.str();
}

static bool h264_needs_level_4_2(const VideoFormat& video_format,
                                 int bitrate_kbits) {
  if (bitrate_kbits >= 20000) return true;
  if (video_format.width <= 640 && video_format.height <= 480 &&
      video_format.framerate <= 90) {
    // <= 480p 4:3 @ 90, level 4.0 enough
    return false;
  }
  if (video_format.width <= 1280 && video_format.height <= 720 &&
      video_format.framerate <= 68) {
    // <= 720p @ 68, level 4.0 enough
    return false;
  }
  if (video_format.width <= 1920 && video_format.height <= 1080 &&
      video_format.framerate <= 30) {
    // <= 1080p @ 30, level 4.0 enough
    return false;
  }
  // exotic or to high, use level 4.2
  return true;
}

static int ALIGN_UP(int value, int multiple) {
  return value + (value % multiple);
}
static int ALIGN(int value, int multiple) { return value + (value % multiple); }

static int rpi_calculate_number_of_mbs_in_a_slice(int frame_height_px,
                                                  int n_slices) {
  if (n_slices < 2) return 0;
  int frame_mb_rows = ALIGN_UP(frame_height_px, 16) / 16;
  if (n_slices > frame_mb_rows) {
    openhd::log::get_default()->warn(
        "Too many slices, frame_mb_rows:%d slices:%d", frame_mb_rows, n_slices);
    return frame_mb_rows;
  }
  int slice_row_mb = frame_mb_rows / n_slices;
  if (frame_mb_rows - n_slices * slice_row_mb)
    slice_row_mb++;  // must round up to avoid extra slice if not evenly divided
  openhd::log::get_default()->debug(
      "frame_height_px:{} n_slices:{} frame_mb_rows:{} slice_row_mb:{}",
      frame_height_px, n_slices, frame_mb_rows, slice_row_mb);
  return slice_row_mb;
}
static int rpi_calculate_intra_refresh_period(int frame_width_px,
                                              int frame_height_px,
                                              int intra_refresh_period) {
  int32_t mbs = 0;
  mbs = ALIGN(frame_width_px, 16) * ALIGN(frame_height_px, 16);
  mbs /= (16 * 16);
  if (mbs % intra_refresh_period) mbs++;
  mbs /= intra_refresh_period;
  openhd::log::get_default()->debug("{}x{} intra_refresh_period:{} mbs:{}",
                                    frame_width_px, frame_height_px,
                                    intra_refresh_period, mbs);
  return mbs;
}

// v4l2 h264 encoder on raspberry pi
// we configure the v4l2 h264 encoder by using the extra controls
// We want constant bitrate (e.g. what the user has set) as long as we don't
// dynamically adjust anything in this regard (video_bitrate_mode) 24.10.22:
// something seems t be bugged on the rpi v4l2 encoder, setting constant bitrate
// doesn't work and somehow increases latency (,video_bitrate_mode=1) The
// default for h264_minimum_qp_value seems to be 20 - we set it to something
// lower, so we can get a higher bitrate on scenes with less change (openhd
// values consistency over everything else)
static std::string create_rpi_v4l2_h264_encoder(
    const CameraSettings& settings) {
  assert(settings.streamed_video_format.videoCodec == VideoCodec::H264);
  // Level wikipedia: https://de.wikipedia.org/wiki/H.264#Level
  // If the level selected is too low, the stream will straight out not start
  // (pi cannot really do more than level 4.0, at least not low latency, but for
  // experimenting, at least make those higher resolutions create a valid
  // pipeline
  std::string rpi_h264_encode_level = "4";
  std::string rpi_h264_encode_level_v4l2_int = "11";
  if (h264_needs_level_4_2(settings.streamed_video_format,
                           settings.h26x_bitrate_kbits)) {
    rpi_h264_encode_level = "4.2";          // Used for gstreamer caps
    rpi_h264_encode_level_v4l2_int = "13";  // Used for 4l2 control
  }
  // NOTE: higher quantization parameter -> lower image quality, and lower
  // bitrate NOTE: The range of QP value is from 0 to 51. Any value more than 51
  // is clamped to 51 RPI Default : 20 / 51
  std::string quantization_str =
      fmt::format(",h264_minimum_qp_value={},h264_maximum_qp_value={}", 5, 51);
  std::string intra_refresh_period_str;
  if (settings.h26x_intra_refresh_type != -1) {
    const int period = rpi_calculate_intra_refresh_period(
        settings.streamed_video_format.width,
        settings.streamed_video_format.height, settings.h26x_keyframe_interval);
    intra_refresh_period_str = fmt::format(",intra_refresh_period={}", period);
  }
  std::string slicing_str;
  if (settings.h26x_num_slices >= 2) {
    const int number_of_mbs_in_a_slice = rpi_calculate_number_of_mbs_in_a_slice(
        settings.streamed_video_format.height, settings.h26x_num_slices);
    slicing_str =
        fmt::format(",number_of_mbs_in_a_slice={}", number_of_mbs_in_a_slice);
  }
  // BUG RPI FOUNDATION: video_bitrate_mode=1 makes encoder non functional
  // rpi v4l2 encoder takes bit/s instead of kbit/s
  const int bitrateBitsPerSecond =
      openhd::kbits_to_bits_per_second(settings.h26x_bitrate_kbits);
  std::string bitrate_str;
  bitrate_str = fmt::format(",video_bitrate={}", bitrateBitsPerSecond);
  std::stringstream ret;
  ret << fmt::format(
      "v4l2h264enc name=rpi_v4l2_encoder "
      "extra-controls=\"controls,repeat_sequence_header=1,h264_profile=1,h264_"
      "level={}{},h264_i_frame_period={},generate_access_unit_delimiters=1{}{}{"
      "}\" ! ",
      rpi_h264_encode_level_v4l2_int, bitrate_str,
      settings.h26x_keyframe_interval, quantization_str,
      intra_refresh_period_str, slicing_str);
  ret << fmt::format(
      "video/x-h264,level=(string){},profile=constrained-baseline ! ",
      rpi_h264_encode_level);
  return ret.str();
}

/**
 * hdmi to csi via (newer) v4l2 driver. Needs matching config.txt. Has issues !
 */
static std::string create_rpi_hdmi_v4l2_stream(const CameraSettings& settings) {
  std::stringstream ss;
  // io-mode=5 ==  GST_V4L2_IO_DMABUF_IMPORT
  // ss << "v4l2src io-mode=5 ! ";
  ss << "v4l2src  io-mode=dmabuf ! ";
  ss << "video/x-raw,framerate=30/1,format=UYVY ! ";
  ss << create_rpi_v4l2_h264_encoder(settings);
  return ss.str();
}

static std::string createLibcamerasrcStream(const CameraSettings& settings) {
  using namespace openhd;
  assert(settings.streamed_video_format.isValid());
  std::stringstream ss;
  // ss << fmt::format("libcamerasrc camera-name={} ",camera_name.value);
  ss << "libcamerasrc ";  // camera_name is not needed for us - we only support
                          // one libcamera at a time. rpi cannot do more than
                          // that anyway.
  // NOTE: those options require openhd/arducam lbcamera !!
  // We make sure not to write them out explicitly when default(s) are still in
  // use
  const auto rotation_degree = libcamera::get_rotation_degree(settings);
  if (rotation_degree.has_value()) {
    ss << "rotation=" << rotation_degree.value() << " ";
  }
  if (requires_hflip(settings)) {
    ss << "hflip=1 ";
  }
  if (requires_vflip(settings)) {
    ss << "vflip=1 ";
  }
  const auto brightness = libcamera::get_brightness(settings);
  if (brightness.has_value()) {
    ss << fmt::format("brightness={} ", brightness.value());
  }
  const auto sharpness = libcamera::get_sharpness(settings);
  if (sharpness.has_value()) {
    ss << fmt::format("sharpness={} ", sharpness.value());
  }
  const auto saturation = libcamera::get_saturation(settings);
  if (saturation.has_value()) {
    ss << fmt::format("saturation={} ", saturation.value());
  }
  const auto contrast = libcamera::get_contrast(settings);
  if (contrast.has_value()) {
    ss << fmt::format("contrast={} ", contrast.value());
  }
  if (openhd::validate_rpi_libcamera_ev_value(
          settings.rpi_libcamera_ev_value) &&
      settings.rpi_libcamera_ev_value != RPI_LIBCAMERA_DEFAULT_EV) {
    ss << fmt::format("ev={} ", settings.rpi_libcamera_ev_value);
  }
  if (openhd::validate_rpi_libcamera_doenise_index(
          settings.rpi_libcamera_denoise_index) &&
      settings.rpi_libcamera_denoise_index != 0) {
    ss << fmt::format("denoise={} ", settings.rpi_libcamera_denoise_index);
  }
  if (openhd::validate_rpi_libcamera_awb_index(
          settings.rpi_libcamera_awb_index) &&
      settings.rpi_libcamera_awb_index != 0) {
    ss << fmt::format("awb={} ", settings.rpi_libcamera_awb_index);
  }
  if (openhd::validate_rpi_libcamera_metering_index(
          settings.rpi_libcamera_metering_index) &&
      settings.rpi_libcamera_metering_index != 0) {
    ss << fmt::format("metering={} ", settings.rpi_libcamera_metering_index);
  }
  if (openhd::validate_rpi_libcamera_exposure_index(
          settings.rpi_libcamera_exposure_index) &&
      settings.rpi_libcamera_exposure_index != 0) {
    ss << fmt::format("exposure={} ", settings.rpi_libcamera_exposure_index);
  }
  if (openhd::validate_rpi_libcamera_shutter_microseconds(
          settings.rpi_libcamera_shutter_microseconds) &&
      settings.rpi_libcamera_shutter_microseconds != 0) {
    ss << fmt::format("shutter={} ",
                      settings.rpi_libcamera_shutter_microseconds);
  }
  // openhd-libcamera specific options end
  ss << " ! ";
  if (settings.streamed_video_format.videoCodec == VideoCodec::H264) {
    // First we set the caps filter(s) on libcamerasrc, this way we control the
    // format (output by ISP), w,h and fps
    ss << fmt::format(
        "capsfilter "
        "caps=video/x-raw,width={},height={},format=NV12,framerate={}/"
        "1,interlace-mode=progressive,colorimetry=bt709 ! ",
        settings.streamed_video_format.width,
        settings.streamed_video_format.height,
        settings.streamed_video_format.framerate);
    if (settings.force_sw_encode) {
      openhd::log::get_default()->warn("Forced SW encode");
      ss << createSwEncoder(settings);
    } else {
      // We got rid of the v4l2convert - see
      // https://github.com/raspberrypi/libcamera/issues/30
      // after the libcamerasrc part, we can just append the rpi v4l2 h264
      // encoder part
      ss << create_rpi_v4l2_h264_encoder(settings);
    }
  } else {
    openhd::log::get_default()->warn(
        "No h265 encoder on rpi, using SW encode (will almost 100% result in "
        "frame drops/performance issues)");
    ss << fmt::format("video/x-raw, width={}, height={}, framerate={}/1 ! ",
                      settings.streamed_video_format.width,
                      settings.streamed_video_format.height,
                      settings.streamed_video_format.framerate);
    ss << createSwEncoder(settings);
  }
  return ss.str();
}

static std::string create_veye_vl2_stream(const CameraSettings& settings,
                                          const std::string& v4l2_device_name) {
  // NOTE: Most veye camera(s) can only do 1080p30, wo this relies on the
  // settings being set to 1080p30 by default for veye camera(s). v4l2src
  // io-mode=dmabuf device=dev/video0 ! video/x-raw,format=(string)UYVY,
  // width=(int)1920, height=(int)1080, framerate=(fraction)30/1 ! ";
  std::stringstream ss;
  ss << fmt::format("v4l2src io-mode=dmabuf device={} ! ", v4l2_device_name);
  ss << fmt::format(
      "video/x-raw,format=(string)UYVY, width={}, height={}, framerate={}/1 ! ",
      settings.streamed_video_format.width,
      settings.streamed_video_format.height,
      settings.streamed_video_format.framerate);
  if (settings.streamed_video_format.videoCodec == VideoCodec::H264) {
    ss << create_rpi_v4l2_h264_encoder(settings);
  } else {
    openhd::log::get_default()->warn(
        "No h265 encoder on rpi, using SW encode (will almost 100% result in "
        "frame drops/performance issues)");
    ss << createSwEncoder(settings);
  }
  return ss.str();
}

static std::string createRockchipEncoderPipeline(
    const CameraSettings& settings) {
  std::stringstream ss;
  const int bps = openhd::kbits_to_bits_per_second(settings.h26x_bitrate_kbits);
  if (settings.streamed_video_format.videoCodec == VideoCodec::H264) {
    ss << "mpph264enc ";
  } else {
    ss << "mpph265enc ";
  }
  ss << "rc-mode=cbr qp-min=10 qp-max=51 bps=" << bps;
  ss << " width=" << settings.streamed_video_format.width;
  ss << " height=" << settings.streamed_video_format.height;
  if (openhd::validate_camera_rotation(settings.camera_rotation_degree)) {
    // Takes 0,90,180,270
    ss << " rotation=" << settings.camera_rotation_degree;
  }
  ss << " gop=" << settings.h26x_keyframe_interval;
  if (h264_needs_level_4_2(settings.streamed_video_format,
                           settings.h26x_bitrate_kbits)) {
    ss << " level=42";
  }
  const int rotation = get_rotation_degree_0_90_180_270(settings);
  ss << " rotation=" << rotation;
  ss << " ! ";
  return ss.str();
}

static std::string createRockchipV4L2Pipeline(const int video_dev,
                                              const int framerate) {
  std::stringstream ss;
  ss << "v4l2src device=/dev/video" << video_dev
     << " io-mode=auto do-timestamp=true ! video/x-raw,format=NV12, ";
  ss << "framerate=" << framerate << "/1 ! ";
  return ss.str();
}

static std::string createRockchipHDMIStream(const CameraSettings& settings) {
  std::stringstream ss;
  ss << createRockchipV4L2Pipeline(0, settings.streamed_video_format.framerate);
  ss << createRockchipEncoderPipeline(settings);
  return ss.str();
}

static std::string createRockchipCSIStream(int v4l2_filenumber,
                                           const CameraSettings& settings) {
  std::stringstream ss;
  ss << createRockchipV4L2Pipeline(v4l2_filenumber,
                                   settings.streamed_video_format.framerate);
  ss << createRockchipEncoderPipeline(settings);
  return ss.str();
}

/**
 * Creates stream for Allwinner camera (v4l2)
 * @param sensor_id sensor id
 * OBSOLETE !
 */
static std::string createAllwinnerSensorPipeline(const int sensor_id,
                                                 const int width,
                                                 const int height,
                                                 const int framerate) {
  std::stringstream ss;
  ss << "v4l2src device=/dev/video" << sensor_id << " ! ";
  ss << "video/x-raw,pixelformat=NV12,";
  ss << "width=" << width << ", ";
  ss << "height=" << height << ", ";
  ss << "framerate=" << framerate << "/1 ! ";
  return ss.str();
}

// using cedar (closed source) HW acceleration.
static std::string createAllwinnerEncoderPipeline(
    const CameraSettings& settings) {
  std::stringstream ss;
  assert(settings.streamed_video_format.videoCodec == VideoCodec::H264);
  ss << "sunxisrc name=sunxisrc bitrate=" << settings.h26x_bitrate_kbits
     << " keyint=" << settings.h26x_keyframe_interval << " ! ";
  return ss.str();
}

/**
 * Create a encoded stream for the allwinner, which is fully hardware
 * accelerated
 */
static std::string createAllwinnerStream(const CameraSettings& settings) {
  std::stringstream ss;
  ss << createAllwinnerEncoderPipeline(settings);
  return ss.str();
}

// Camera quirks, omit arguments when set to 0 - some cameras refuse to work
// even though the correct width, height or fps is given
static std::string gst_v4l2_width_height_fps_unless_omit(
    const CameraSettings& settings) {
  std::stringstream ss;
  if (settings.streamed_video_format.width > 0) {
    ss << fmt::format(", width={}", settings.streamed_video_format.width);
  }
  if (settings.streamed_video_format.height > 0) {
    ss << fmt::format(", height={}", settings.streamed_video_format.height);
  }
  if (settings.streamed_video_format.framerate > 0) {
    ss << fmt::format(", framerate={}/1",
                      settings.streamed_video_format.framerate);
  }
  return ss.str();
}

/**
 * For V4l2 Cameras that do raw YUV (or RGB) we use a sw encoder.
 * This one has no custom resolution(s) yet.
 */
static std::string createV4l2SrcRawAndSwEncodeStream(
    const std::string& device_node, const CameraSettings& settings) {
  std::stringstream ss;
  ss << fmt::format("v4l2src device={} ! ", device_node);
  ss << "video/x-raw";
  ss << gst_v4l2_width_height_fps_unless_omit(settings);
  ss << " ! ";
  ss << "videoconvert ! ";
  // Add a queue here. With sw we are not low latency anyways.
  ss << "queue ! ";
  // For some reason gstreamer can't automatically figure things out here
  ss << "video/x-raw, format=I420 ! ";
  ss << createSwEncoder(settings);
  return ss.str();
}
// ------------- crateXXXStream end  -------------

/**
 * Create the part of the pipeline that takes the raw h264/h265/mjpeg from
 * gstreamer and packs it into rtp.
 * @param videoCodec the video codec to create the rtp for.
 * @return the gstreamer pipeline part.
 */
static std::string create_parse_and_rtp_packetize(
    const VideoCodec videoCodec, int rtp_fragment_size = 1024) {
  std::stringstream ss;
  ss << "queue ! ";
  ss << create_parse_for_codec(videoCodec);
  ss << create_rtp_packetize_for_codec(videoCodec, rtp_fragment_size);
  return ss.str();
}
static std::string create_queue_and_parse(const VideoCodec videoCodec) {
  std::stringstream ss;
  ss << "queue ! ";
  ss << create_parse_for_codec(videoCodec);
  return ss.str();
}

static std::string create_caps_nal(const VideoCodec& videoCodec,
                                   bool alignment_nal) {
  if (videoCodec == VideoCodec::H264) {
    if (alignment_nal) {
      return "video/x-h264,stream-format=byte-stream,alignment=nal ! ";
    } else {
      return "video/x-h264,stream-format=byte-stream,alignment=au ! ";
    }
  }
  return "video/x-h265, stream-format=\"byte-stream\" ! ";
}

/**
 * Create the part of the pipeline that takes the rtp from gstreamer and sends
 * it to udp.
 * @param udpOutPort the udp (localhost) port.
 * @return the gstreamer pipeline part
 */
static std::string createOutputUdpLocalhost(const int udpOutPort) {
  return fmt::format(" udpsink host=127.0.0.1 port={}", udpOutPort);
}

static std::string createOutputAppSink() {
  // @wait-on-eos: set to false since when terminating, we don't care if all
  // buffers of appsink have been consumed or not.
  return " appsink drop=true name=out_appsink wait-on-eos=false";
}

// Needs to match below
static std::string file_suffix_for_video_codec(const VideoCodec videoCodec) {
  if (videoCodec == VideoCodec::H264 || videoCodec == VideoCodec::H265) {
    return ".mkv";
  } else {
    return ".avi";
  }
}
// Assumes there is a tee command named "t" somewhere in the pipeline right
// after the encoding step, so we can get the raw encoded data out.
// .avi supports h264 and mjpeg, and works even in case the stream would crash
// doesn't support h265 though
// .mp4 is always corrupted on crash
// .mkv supports h264 and h265, but no mjpeg. It is the default in OBS though,
// so we decided to use .mkv for h264 and h265 and .avi for mjpeg in case the
// gst pipeline is not stopped properly
static std::string createRecordingForVideoCodec(
    const VideoCodec videoCodec, const std::string& out_filename) {
  std::stringstream ss;
  // don't forget the white space before the " t." !
  ss << " t. ! queue ! ";
  if (videoCodec == VideoCodec::H264) {
    ss << "h264parse ! ";
  } else if (videoCodec == VideoCodec::H265) {
    ss << "h265parse ! ";
  }
  // ss <<"mp4mux ! filesink location="<<out_filename;
  if (videoCodec == VideoCodec::H264 || videoCodec == VideoCodec::H265) {
    ss << "matroskamux ! filesink location=" << out_filename;
  }
  return ss.str();
}

static std::string create_input_custom_udp_rtp_port(
    const CameraSettings& settings) {
  static constexpr auto input_port = 5500;
  static constexpr auto address = "127.0.0.1";
  std::stringstream ss;
  ss << fmt::format(
      "udpsrc address={} port={} {} ! ", address, input_port,
      gst_create_rtp_caps(settings.streamed_video_format.videoCodec));
  ss << create_rtp_depacketize_for_codec(
      settings.streamed_video_format.videoCodec);
  return ss.str();
}

// Dummy stream using either HW or SW encode.
static std::string createDummyStreamX(const CameraSettings& settings) {
  const auto platform = OHDPlatform::instance();
  std::stringstream ss;
  ss << "videotestsrc name=videotestsrc ! ";
  // h265 cannot do NV12, but I420.
  // x264 can do both NV12 and I420
  // so we use I420 here since every SW encoder can do it.
  ss << fmt::format(
      "video/x-raw, format=I420,width={},height={},framerate={}/1 ! ",
      settings.streamed_video_format.width,
      settings.streamed_video_format.height,
      settings.streamed_video_format.framerate);
  if (settings.force_sw_encode) {
    ss << createSwEncoder(settings);
  } else {
    if (platform.is_rpi()) {
      ss << create_rpi_v4l2_h264_encoder(settings);
    } else if (platform.is_rock()) {
      ss << createRockchipEncoderPipeline(settings);
    } else {
      ss << createSwEncoder(settings);
    }
  }
  // since the primary purpose here is testing, use sw encoder, which is always
  // guaranteed to work
  // ss << createSwEncoder(settings);
  return ss.str();
}

static std::string create_dummy_filesrc_stream(const CameraSettings& settings) {
  const auto platform = OHDPlatform::instance();
  auto files = OHDFilesystemUtil::getAllEntriesFullPathInDirectory(
      "/usr/local/share/openhd/dev");
  // std::string filename="/usr/local/share/openhd/dev/test.mp4";
  // if(!files.empty())filename=files.at(0);
  // const std::string PATH="/usr/local/share/openhd/dev/";
  std::string PATH = "/home/openhd/";
  if (OHDFilesystemUtil::exists("/home/consti10/Desktop/intra_test/out/")) {
    PATH = "/home/consti10/Desktop/intra_test/out/";
  }
  std::string filename = "";
  const int width_px = settings.streamed_video_format.width;
  if (width_px >= 1920) {
    filename = "wing_1920x1080p60.mkv";
  } else if (width_px >= 1280) {
    filename = "wing_1280x720p60.mkv";
  } else {
    filename = "wing_848x480p60.mkv";
  }
  const std::string path_filename = PATH + filename;
  std::stringstream ss;
  // ss<<"multifilesrc location="<<filename<<" loop=true ! ";
  ss << "filesrc location=" << path_filename << " ! ";
  if (OHDPlatform::instance().platform_type == X_PLATFORM_TYPE_X86) {
    // decodebin sometimes doesn't work for whatever reason
    // ss << "x264dec ! ";
    ss << "decodebin force-sw-decoders=true ! ";
  } else {
    ss << "decodebin ! ";
  }
  ss << fmt::format("videorate max-rate={} ! ",
                    settings.streamed_video_format.framerate);
  ss << "queue ! ";
  ss << "videoscale ! ";
  if (platform.is_rock()) {
    // ROCK needs NV12
    ss << "video/x-raw, format=NV12";
    ss << fmt::format(",width={}, height={}, framerate={}/1 ! ",
                      settings.streamed_video_format.width,
                      settings.streamed_video_format.height,
                      settings.streamed_video_format.framerate);
  } else {
    ss << "video/x-raw, format=I420";
    ss << fmt::format(",width={}, height={}, framerate={}/1 ! ",
                      settings.streamed_video_format.width,
                      settings.streamed_video_format.height,
                      settings.streamed_video_format.framerate);
  }
  // ss<<createSwEncoder(settings);
  if (settings.force_sw_encode) {
    ss << createSwEncoder(settings);
  } else {
    if (platform.is_rpi()) {
      if (settings.streamed_video_format.videoCodec == VideoCodec::H264) {
        ss << create_rpi_v4l2_h264_encoder(settings);
      } else {
        ss << createSwEncoder(settings);
      }
    } else if (platform.is_rock()) {
      ss << createRockchipEncoderPipeline(settings);
    } else {
      ss << createSwEncoder(settings);
    }
  }
  // ss<<"qtdemux ! queue ! h264parse config-interval=-1 !
  // video/x-h264,stream-format=byte-stream,alignment=au !";
  return ss.str();
}

static std::string create_nvarguscamerasrc(const CameraSettings& settings) {
  std::stringstream ss;
  ss << "nvarguscamerasrc do-timestamp=true ! ";
  ss << "video/x-raw(memory:NVMM), format=NV12, ";
  ss << "width=" << settings.streamed_video_format.width << ", ";
  ss << "height=" << settings.streamed_video_format.height << ", ";
  ss << "framerate=" << settings.streamed_video_format.framerate << "/1 ! ";
  return ss.str();
}
// For jetson we have a nice separation between camera / ISP and encoder
// This creates the encoding part of the gstreamer pipeline
static std::string create_nvidia_encoder_pipeline(
    const CameraSettings& settings) {
  std::stringstream ss;
  // Consti10: I would like to use the nvv4l2h264/5encoder, but r.n it looks to
  // me as if the stream created by it cannot be decoded properly by multiple
  // platform(s). With the omxh26Xdec equivalents, this issue does not exist.
  // UPDATE: It is even more complicated. For h264, we need to use omx encode
  // and nvv4 decode For h265, we need to use nvv4 encode and nvv4 decode
  // https://developer.download.nvidia.com/embedded/L4T/r31_Release_v1.0/Docs/Accelerated_GStreamer_User_Guide.pdf?E_vSS50FKrZaJBjDtnCBmtaY8hWM1QCYlMHtXBqvZ_Jeuw0GXuLNaQwMBWUDABSnWCD-p8ABlBpBpP-kb2ADgWugbW8mgGPxUWJG_C4DWaL1yKjUVMy1AxH1RTaGOW82yFJ549mea--FBPuZUH3TT1MoEd4-sgdrZal5qr1J0McEFeFaVUc&t=eyJscyI6InJlZiIsImxzZCI6IlJFRi1kb2NzLm52aWRpYS5jb21cLyJ9
  // jetson is also bits per second
  const auto bitrateBitsPerSecond =
      openhd::kbits_to_bits_per_second(settings.h26x_bitrate_kbits);
  if (settings.streamed_video_format.videoCodec == VideoCodec::H264) {
    const bool use_omx_encoder = true;
    if (use_omx_encoder) {
      // for omx control-rate=2 means constant, in constrast to nvv4l2h264enc
      ss << "omxh264enc control-rate=2 insert-sps-pps=true bitrate="
         << bitrateBitsPerSecond << " ";
      ss << "iframeinterval=" << settings.h26x_keyframe_interval << " ";
      // this was added to test if it fixes issues when decoding jetson h264 on
      // rpi
      ss << "insert-vui=true ";
      ss << "! ";
    } else {
      ss << "nvv4l2h264enc control-rate=1 insert-sps-pps=true bitrate="
         << bitrateBitsPerSecond << " ";
      ss << "iframeinterval=" << settings.h26x_keyframe_interval << " ";
      // https://forums.developer.nvidia.com/t/high-decoding-latency-for-stream-produced-by-nvv4l2h264enc-compared-to-omxh264enc/159517
      // https://forums.developer.nvidia.com/t/parameter-poc-type-missing-on-jetson-though-mentioned-in-the-documentation/164545
      // NOTE: This is quite important, without it jetson nvv4l2decoder cannot
      // properly decode (confirmed) (WTF nvidia you cannot encode your own data
      // haha ;) ) As well as a lot of phones (confirmed) and perhaps the
      // rpi,too (not confirmed).
      ss << "poc-type=2 ";
      // TODO should we make max-perf-enable on by default ?
      ss << "maxperf-enable=true ";
      ss << "! ";
    }
  } else if (settings.streamed_video_format.videoCodec == VideoCodec::H265) {
    const bool use_omx_encoder = false;
    if (use_omx_encoder) {
      // for omx control-rate=2 means constant, in contrast to nvv4l2h264enc
      ss << "omxh265enc control-rate=2 insert-sps-pps=true bitrate="
         << bitrateBitsPerSecond << " ";
      ss << "iframeinterval=" << settings.h26x_keyframe_interval << " ";
      ss << "! ";
    } else {
      ss << "nvv4l2h265enc control-rate=1 insert-sps-pps=true bitrate="
         << bitrateBitsPerSecond << " ";
      // TODO what is the difference between iframeinterval and idrinterval
      ss << "iframeinterval=" << settings.h26x_keyframe_interval << " ";
      ss << "idrinterval=" << settings.h26x_keyframe_interval << " ";
      ss << "maxperf-enable=true ";
      ss << "! ";
    }
  } else {
    assert(false);
  }
  return ss.str();
}

static std::string create_nvidia_xavier_stream(const CameraSettings& settings) {
  std::stringstream ss;
  ss << create_nvarguscamerasrc(settings);
  ss << create_nvidia_encoder_pipeline(settings);
  return ss.str();
}

}  // namespace OHDGstHelper
#endif  // OPENHD_OHDGSTHELPER_H
