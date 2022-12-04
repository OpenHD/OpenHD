//
// Created by consti10 on 15.05.22.
//

#ifndef OPENHD_OHDGSTHELPER_H
#define OPENHD_OHDGSTHELPER_H

// NOTE: Spdlog uses format internally, when pulling in fmt first and then spdlog we can have compiler issues
#include "openhd-spdlog.hpp"
//#include <fmt/format.h>
#include <gst/gst.h>

#include <sstream>
#include <string>

#include "camera_settings.hpp"

/**
 * Helper methods to create parts of gstreamer pipes.
 * Note: Unless a pipeline part is used at the end, all pipelines should end
 * with '! '. This way we avoid syntax errors.
 */
namespace OHDGstHelper {

// These most basic params are supported on pretty much any platform / every good encoder implementation
struct CommonEncoderParams{
  VideoCodec videoCodec;
  // For h264/h265 only. MJPEG in general only supports a "quality" param, not bitrate(s)
  int h26X_bitrate_kbits;
  // For h264/h265 only, often also called key-int-max or similar.
  int h26X_keyframe_interval;
  // for MJPEG only, usually in a [0,100] range
  int mjpeg_quality_percent;
};

static CommonEncoderParams extract_common_encoder_params(const CameraSettings& settings){
  return {.videoCodec=settings.streamed_video_format.videoCodec,.h26X_bitrate_kbits=settings.h26x_bitrate_kbits,
  .h26X_keyframe_interval=settings.h26x_keyframe_interval,.mjpeg_quality_percent=settings.mjpeg_quality_percent};
}

/**
 * Check if we can find gstreamer at run time, throw a runtime error if not.
 */
static void initGstreamerOrThrow() {
  GError *error = nullptr;
  if (!gst_init_check(nullptr, nullptr, &error)) {
    openhd::log::get_default()->error("gst_init_check() failed: {}",error->message);
    g_error_free(error);
    throw std::runtime_error("GStreamer initialization failed");
  }
}

// SW encoding is slow, but should work on all platforms (at least for low resolutions/framerate(s) )
// Note that not every sw encoder accepts every type of input format - I420 seems to work for all of them though.
static std::string createSwEncoder(const CommonEncoderParams& common_encoder_params){
  std::stringstream ss;
  if(common_encoder_params.videoCodec==VideoCodec::H264){
	ss << "x264enc name=swencoder bitrate=" << common_encoder_params.h26X_bitrate_kbits <<
	" speed-preset=ultrafast"<<
	" tune=zerolatency key-int-max=" << common_encoder_params.h26X_keyframe_interval <<
        " sliced-threads=0"<< //Note: Sliced threads has some advantages, but r.n it is incompatible with QOpenHD on a pi (decode)
        " ! ";
  }else if(common_encoder_params.videoCodec==VideoCodec::H265){
	//TODO: jetson sw encoder (x265enc) is so old it doesn't have the key-int-max param
	ss << "x265enc name=swencoder bitrate=" << common_encoder_params.h26X_bitrate_kbits <<
	" speed-preset=ultrafast"<<
	" tune=zerolatency key-int-max=" << common_encoder_params.h26X_keyframe_interval << " ! ";
  }else{
	assert(common_encoder_params.videoCodec==VideoCodec::MJPEG);
	//NOTE jpegenc doesn't have a bitrate controll
	ss<<"jpegenc quality="<<common_encoder_params.mjpeg_quality_percent<< " ! ";
  }
  return ss.str();
}

// a createXXXStream function always ends wth an encoded "h164,h265 or mjpeg
// stream ! " aka after that, onc can add a rtp encoder or similar. All these
// methods also start from zero - aka have a source like videotestsrc,
// nvarguscamerasr usw in the beginning and end with a OpenHD supported video
// codec (e.g. h264,h265 or mjpeg)
// ------------- crateXXXStream begin -------------
/**
 * Create a encoded dummy stream for the selected video format, that means a
 * stream that takes raw data coming from a videotestsrc and encodes it in
 * either h264, h265 or mjpeg.
 */
static std::string createDummyStream(const CameraSettings& settings) {
  std::stringstream ss;
  ss << "videotestsrc ! ";
  // h265 cannot do NV12, but I420.
  // x264 and mjpeg can do both NV12 and I420
  // so we use I420 here since every SW encoder can do it.
  ss << fmt::format(
      "video/x-raw, format=I420,width={},height={},framerate={}/1 ! ",
      settings.streamed_video_format.width, settings.streamed_video_format.height, settings.streamed_video_format.framerate);
  // since the primary purpose here is testing, use a fixed low key frame
  // interval.
  ss << createSwEncoder(extract_common_encoder_params(settings));
  return ss.str();
}

/**
 * Create a encoded stream for rpicamsrc, which supports h264 only.
 * @param camera_number use -1 to let rpicamsrc decide
 * See https://gstreamer.freedesktop.org/documentation/rpicamsrc/index.html?gi-language=c#GstRpiCamSrcAWBMode for more complicated params
 */
static std::string createRpicamsrcStream(const int camera_number,
                                         const CameraSettings& settings) {
  assert(settings.streamed_video_format.isValid());
  //assert(videoFormat.videoCodec == VideoCodec::H264);
  std::stringstream ss;
  // other than the other ones, rpicamsrc takes bit/s instead of kbit/s
  const int bitrateBitsPerSecond = kbits_to_bits_per_second(settings.h26x_bitrate_kbits);
  if (camera_number == -1) {
    ss << fmt::format("rpicamsrc name=rpicamsrc bitrate={} preview=0 ",
                      bitrateBitsPerSecond);
  } else {
    ss << fmt::format("rpicamsrc name=rpicamsrc camera-number={} bitrate={} preview=0 ",
                      camera_number, bitrateBitsPerSecond);
  }
  // keyframe-interval   : Interval (in frames) between I frames. -1 = automatic, 0 = single-keyframe
  if(openhd::validate_rpi_keyframe_interval(settings.h26x_keyframe_interval)){
    ss << "keyframe-interval="<<settings.h26x_keyframe_interval <<" ";
  }
  if(openhd::validate_rpi_intra_refresh_type(settings.h26x_intra_refresh_type)){
    ss << "intra-refresh-type="<<settings.h26x_intra_refresh_type <<" ";
  }
  if(openhd::validate_camera_rotation(settings.camera_rotation_degree)){
    ss<<"rotation="<<settings.camera_rotation_degree<<" ";
  }
  if(settings.horizontal_flip){
    ss<<"hflip=1 ";
  }
  if(settings.vertical_flip){
    ss<<"vflip=1 ";
  }
  if(openhd::validate_rpi_awb_mode(settings.awb_mode)){
    ss<<"awb-mode="<<settings.awb_mode<<" ";
  }
  if(openhd::validate_rpi_exp_mode(settings.exposure_mode)){
    ss<<"exposure-mode="<<settings.exposure_mode<<" ";
  }
  if(openhd::validate_rpi_brightness(settings.brightness_percentage)){
    ss<<"brightness="<<settings.brightness_percentage<<" ";
  }
  if(openhd::validate_rpi_rpicamsrc_iso(settings.rpi_rpicamsrc_iso)){
    ss<<"iso="<<settings.rpi_rpicamsrc_iso<<" ";
  }
  // Note: ROI (Region of interest) on the rpi does not tell the encoder to allocate more bandwidth at a specific area,
  // but rather zooms in on a specific area (which is not really of use to use)
  ss<<" ! ";
  if(settings.streamed_video_format.videoCodec==VideoCodec::H264){
	ss << fmt::format(
		"video/x-h264, profile=constrained-baseline, width={}, height={}, "
		"framerate={}/1, level=3.0 ! ",
        settings.streamed_video_format.width, settings.streamed_video_format.height, settings.streamed_video_format.framerate);
  }else{
	openhd::log::get_default()->warn("No h265 / MJPEG encoder on rpi, using SW encode (might result in frame drops/performance issues");
	ss<<fmt::format(
		"video/x-raw, width={}, height={}, framerate={}/1 ! ",
            settings.streamed_video_format.width, settings.streamed_video_format.height, settings.streamed_video_format.framerate);
	ss<<createSwEncoder(extract_common_encoder_params(settings));
  }
  return ss.str();
}

static std::string createLibcamerasrcStream(const std::string& camera_name,
                                         const CameraSettings& settings) {
  assert(settings.streamed_video_format.isValid());
  std::stringstream ss;
  // other than the other ones, rpicamsrc takes bit/s instead of kbit/s
  const int bitrateBitsPerSecond = kbits_to_bits_per_second(settings.h26x_bitrate_kbits);
  ss << fmt::format("libcamerasrc camera-name={}",
                      camera_name);
  ss << " ! ";
  if (settings.streamed_video_format.videoCodec == VideoCodec::H264) {
    // First we set the caps filter(s) on libcamerasrc, this way we control the format (output by ISP), w,h and fps
    ss << fmt::format(
        "capsfilter caps=video/x-raw,width={},height={},format=NV12,framerate={}/1,interlace-mode=progressive,colorimetry=bt709 ! ",
        settings.streamed_video_format.width, settings.streamed_video_format.height, settings.streamed_video_format.framerate);
    // We got rid of the v4l2convert - see
    // https://github.com/raspberrypi/libcamera/issues/30
    // and configure the v4l2 h264 encoder by using the extra controls
    // We want constant bitrate (e.g. what the user has set) as long as we don't dynamcially adjust anything
    // in this regard (video_bitrate_mode)
    // 24.10.22: something seems t be bugged on the rpi v4l2 encoder, setting constant bitrate doesn't work and
    // somehow increases latency (,video_bitrate_mode=1)
    // The default for h264_minimum_qp_value seems to be 20 - we set it to something lower, so we can get a higher bitrate
    // on scenes with less change
    static constexpr auto OPENHD_H264_MIN_QP_VALUE=10;
    ss << fmt::format("v4l2h264enc extra-controls=\"controls,repeat_sequence_header=1,h264_profile=1,h264_level=11,video_bitrate={},h264_i_frame_period={},h264_minimum_qp_value={}\" ! "
        "video/x-h264,level=(string)4 ! ",bitrateBitsPerSecond,settings.h26x_keyframe_interval,OPENHD_H264_MIN_QP_VALUE);
  } else if (settings.streamed_video_format.videoCodec == VideoCodec::MJPEG) {
    ss << fmt::format(
        "capsfilter caps=video/x-raw,width={},height={},format=YVYU,framerate={}/1,interlace-mode=progressive,colorimetry=bt709 ! ",
        settings.streamed_video_format.width, settings.streamed_video_format.height, settings.streamed_video_format.framerate);
    ss << fmt::format("v4l2jpegenc extra-controls=\"controls,compression_quality={}\" ! ",50); //mjpeg has a compression quality not bitrate
  }
  else {
    openhd::log::get_default()->warn("No h265 encoder on rpi, using SW encode (will almost 100% result in frame drops/performance issues)");
    ss << fmt::format("video/x-raw, width={}, height={}, framerate={}/1 ! ",
                      settings.streamed_video_format.width, settings.streamed_video_format.height,
                      settings.streamed_video_format.framerate);
    ss << createSwEncoder(extract_common_encoder_params(settings));
  }
  return ss.str();
}

// For jetson we have a nice separation between camera / ISP and encoder
// This creates the encoding part of the gstreamer pipeline
static std::string createJetsonEncoderPipeline(const CommonEncoderParams& common_encoder_params){
  std::stringstream ss;
  // Consti10: I would like to use the nvv4l2h264/5encoder, but r.n it looks to me as if
  // the stream created by it cannot be decoded properly by multiple platform(s). With the omxh26Xdec equivalents,
  // this issue does not exist.
  // UPDATE: It is even more complicated. For h264, we need to use omx encode and nvv4 decode
  // For h265, we need to use nvv4 encode and nvv4 decode
  // https://developer.download.nvidia.com/embedded/L4T/r31_Release_v1.0/Docs/Accelerated_GStreamer_User_Guide.pdf?E_vSS50FKrZaJBjDtnCBmtaY8hWM1QCYlMHtXBqvZ_Jeuw0GXuLNaQwMBWUDABSnWCD-p8ABlBpBpP-kb2ADgWugbW8mgGPxUWJG_C4DWaL1yKjUVMy1AxH1RTaGOW82yFJ549mea--FBPuZUH3TT1MoEd4-sgdrZal5qr1J0McEFeFaVUc&t=eyJscyI6InJlZiIsImxzZCI6IlJFRi1kb2NzLm52aWRpYS5jb21cLyJ9
  // jetson is also bits per second
  const auto bitrateBitsPerSecond =kbits_to_bits_per_second(common_encoder_params.h26X_bitrate_kbits);
  if(common_encoder_params.videoCodec==VideoCodec::H264){
    const bool use_omx_encoder= true;
    if(use_omx_encoder){
      // for omx control-rate=2 means constant, in constrast to nvv4l2h264enc
      ss<<"omxh264enc control-rate=2 insert-sps-pps=true bitrate="<<bitrateBitsPerSecond<<" ";
      ss<<"iframeinterval="<<common_encoder_params.h26X_keyframe_interval<<" ";
      // this was added to test if it fixes issues when decoding jetson h264 on rpi
      ss<<"insert-vui=true ";
      ss<<"! ";
    }else{
      ss<<"nvv4l2h264enc control-rate=1 insert-sps-pps=true bitrate="<<bitrateBitsPerSecond<<" ";
      ss<<"iframeinterval="<<common_encoder_params.h26X_keyframe_interval<<" ";
      // https://forums.developer.nvidia.com/t/high-decoding-latency-for-stream-produced-by-nvv4l2h264enc-compared-to-omxh264enc/159517
      // https://forums.developer.nvidia.com/t/parameter-poc-type-missing-on-jetson-though-mentioned-in-the-documentation/164545
      // NOTE: This is quite important, without it jetson nvv4l2decoder cannot properly decode (confirmed) (WTF nvidia you cannot encode your own data haha ;) )
      // As well as a lot of phones (confirmed) and perhaps the rpi,too (not confirmed).
      ss<<"poc-type=2 ";
      // TODO should we make max-perf-enable on by default ?
      ss<<"maxperf-enable=true ";
      ss<<"! ";
    }
  }else if(common_encoder_params.videoCodec==VideoCodec::H265){
	  const bool use_omx_encoder= false;
    if(use_omx_encoder){
      // for omx control-rate=2 means constant, in contrast to nvv4l2h264enc
      ss<<"omxh265enc control-rate=2 insert-sps-pps=true bitrate="<<bitrateBitsPerSecond<<" ";
      ss<<"iframeinterval="<<common_encoder_params.h26X_keyframe_interval<<" ";
      ss<<"! ";
    }else{
      ss<<"nvv4l2h265enc control-rate=1 insert-sps-pps=true bitrate="<<bitrateBitsPerSecond<<" ";
      // TODO what is the difference between iframeinterval and idrinterval
      ss<<"iframeinterval="<<common_encoder_params.h26X_keyframe_interval<<" ";
      ss<<"idrinterval="<<common_encoder_params.h26X_keyframe_interval<<" ";
      ss<<"maxperf-enable=true ";
      ss<<"! ";
    }
  }else{
    assert(common_encoder_params.videoCodec==VideoCodec::MJPEG);
    ss<<"nvjpegenc quality="<<common_encoder_params.mjpeg_quality_percent<<" ";
    ss<<"! ";
  }
  return ss.str();
}
/**
 * This creates the sensor/ISP (nvarguscamerasrc) part of the gstreamer pipeline.
 * @param sensor_id sensor id, set to -1 to let nvarguscamerasrc figure it out
 */
static std::string createJetsonSensorPipeline(const int sensor_id,const int width,const int height,const int framerate){
  std::stringstream ss;
  // possible to omit the sensor id, nvarguscamerasrc will then figure out the
  // right sensor id. This only works with one csi camera though.
  if (sensor_id == -1) {
	  ss << "nvarguscamerasrc do-timestamp=true ! ";
  } else {
	  ss<<"nvarguscamerasrc do-timestamp=true sensor-id="<<sensor_id<<" ! ";
  }
  ss<<"video/x-raw(memory:NVMM), format=NV12, ";
  ss<<"width="<<width<<", ";
  ss<<"height="<<height<<", ";
  ss<<"framerate="<<framerate<<"/1 ! ";
  return ss.str();
}
/**
 * Create a encoded stream for the jetson, which is fully hardware accelerated
 * for h264,h265 and mjpeg.
 */
static std::string createJetsonStream(const int sensor_id,
                                      const CameraSettings& settings) {
  std::stringstream ss;
  ss<<createJetsonSensorPipeline(sensor_id,settings.streamed_video_format.width,settings.streamed_video_format.height,settings.streamed_video_format.framerate);
  ss<<createJetsonEncoderPipeline(extract_common_encoder_params(settings));
  return ss.str();
}

static std::string createRockchipEncoderPipeline(const int width, const int height, const CommonEncoderParams& encoder_params){
  std::stringstream ss;
  const int bps = kbits_to_bits_per_second(encoder_params.h26X_bitrate_kbits);
  if(encoder_params.videoCodec==VideoCodec::H264){
    ss<<"mpph264enc rc-mode=cbr bps="<<bps;
    ss<<" width="<<width;
    ss<<" height="<<height;
    ss<<" gop="<<encoder_params.h26X_keyframe_interval<<" ! ";
  }else if(encoder_params.videoCodec==VideoCodec::H265){
    ss<<"mpph264enc rc-mode=cbr bps="<<bps;
    ss<<" width="<<width;
    ss<<" height="<<height;
    ss<<" gop="<<encoder_params.h26X_keyframe_interval<<" ! ";
  }else{
    assert(encoder_params.videoCodec==VideoCodec::MJPEG);
    ss<<"mppjpegenc rc-mode=fixqp quant="<<(encoder_params.mjpeg_quality_percent/10);
    ss<<" width="<<width;
    ss<<" height="<<height;
    ss<<" ! ";
  }
  return ss.str();
}

static std::string createRockchipRecordingPipeline(const int width, const int height, const CommonEncoderParams& encoder_params){
  std::stringstream ss;
  const int bps = kbits_to_bits_per_second(encoder_params.h26X_bitrate_kbits);
  ss<<"tee name=o ! ";
  if(encoder_params.videoCodec==VideoCodec::H264){
    ss<<"mpph264enc rc-mode=cbr bps="<<bps;
    ss<<" width="<<width;
    ss<<" height="<<height;
  }else if(encoder_params.videoCodec==VideoCodec::H265){
    ss<<"mpph264enc rc-mode=cbr bps="<<bps;
    ss<<" width="<<width;
    ss<<" height="<<height;
  }else{
    assert(encoder_params.videoCodec==VideoCodec::MJPEG);
    ss<<"mppjpegenc rc-mode=fixqp quant="<<(encoder_params.mjpeg_quality_percent/10);
    ss<<" width="<<width;
    ss<<" height="<<height;
  }
  ss<<" name=t o. ! ";
  return ss.str();
}

static std::string createRockchipV4L2Pipeline(const int video_dev, const int framerate){
  std::stringstream ss;
  ss<<"v4l2src device=/dev/video"<<video_dev<<" io-mode=auto do-timestamp=true ! video/x-raw,";
  ss<<"framerate="<<framerate<<"/1 ! ";
  return ss.str();
}

static std::string createRockchipHDMIStream(
  bool recording,
  const int bitrateKBits,
  const VideoFormat videoFormat,
  const VideoFormat recordingFormat,
  const int keyframe_interval
) {
  std::stringstream ss;
  ss<<createRockchipV4L2Pipeline(0, videoFormat.framerate);
  if(recording) ss<<createRockchipRecordingPipeline(recordingFormat.width, recordingFormat.height, {recordingFormat.videoCodec, bitrateKBits, keyframe_interval,50});
  ss<<createRockchipEncoderPipeline(videoFormat.width, videoFormat.height, {videoFormat.videoCodec, bitrateKBits, keyframe_interval,50});
  return ss.str();
}

/**
 * For V4l2 Cameras that do raw YUV (or RGB) we use a sw encoder.
 * This one has no custom resolution(s) yet.
 */
static std::string createV4l2SrcRawAndSwEncodeStream(
    const std::string &device_node, const VideoCodec videoCodec,
    const int bitrateKBits,const int keyframe_interval) {
  std::stringstream ss;
  ss << fmt::format("v4l2src device={} ! ", device_node);
  // rn we omit the set resolution/framerate here and let gstreamer figure it
  // out.
  // TODO: do it better ;)
  //openhd::loggers::get_default()->warn("Allowing gstreamer to choose UVC format");
  ss << fmt::format("video/x-raw ! ");
  ss << "videoconvert ! ";
  // Add a queue here. With sw we are not low latency anyways.
  ss << "queue ! ";
  // For some reason gstreamer can't automatically figure things out here
  ss<<"video/x-raw, format=I420 ! ";
  ss<<createSwEncoder({videoCodec,bitrateKBits,keyframe_interval,50});
  return ss.str();
}

/**
 * This one is for v4l2src cameras that outputs already encoded video.
 */
static std::string createV4l2SrcAlreadyEncodedStream(
    const std::string &device_node, const VideoFormat videoFormat) {
  std::stringstream ss;
  ss << fmt::format("v4l2src device={} ! ", device_node);
  if (videoFormat.videoCodec == VideoCodec::H264) {
    ss << fmt::format("video/x-h264, width={}, height={}, framerate={}/1 ! ",
                      videoFormat.width, videoFormat.height,
                      videoFormat.framerate);
  } else if (videoFormat.videoCodec == VideoCodec::H265) {
    ss << fmt::format("video/x-h265, width={}, height={}, framerate={}/1 ! ",
                      videoFormat.width, videoFormat.height,
                      videoFormat.framerate);
  } else {
    assert(videoFormat.videoCodec == VideoCodec::MJPEG);
    ss << fmt::format("image/jpeg, width={}, height={}, framerate={}/1 ! ",
                      videoFormat.width, videoFormat.height,
                      videoFormat.framerate);
  }
  return ss.str();
}

// These are not tested
static std::string createUVCH264Stream(const std::string &device_node,
                                       const int bitrateKBits,
                                       const VideoFormat videoFormat) {
  assert(videoFormat.videoCodec == VideoCodec::H264);
  // https://gstreamer.freedesktop.org/documentation/uvch264/uvch264src.html?gi-language=c#uvch264src:average-bitrate
  // bitrate in bits per second
  const int bitrateBitsPerSecond = kbits_to_bits_per_second(bitrateKBits);
  std::stringstream ss;
  ss << fmt::format(
      "uvch264src device={} peak-bitrate={} initial-bitrate={} "
      "average-bitrate={} rate-control=1 iframe-period=1000 "
      "auto-start=true encodectrl.vidsrc ! ",
      device_node, bitrateBitsPerSecond, bitrateBitsPerSecond,
      bitrateBitsPerSecond);
  ss << fmt::format("video/x-h264,width={}, height={}, framerate={}/1 ! ",
                    videoFormat.width, videoFormat.height,
                    videoFormat.framerate);
  return ss.str();
}
static std::string createIpCameraStream(const std::string &url) {
  std::stringstream ss;
  // none of the other params are used at the moment, we would need to set them
  // with ONVIF or a per-camera API of some sort, however most people seem to
  // set them in advance with the proprietary app that came with the camera
  // anyway
  ss << fmt::format("rtspsrc location=\"{}\" latency=0 ! ", url);
  return ss.str();
}
// ------------- crateXXXStream end  -------------

/**
 * Create the part of the pipeline that takes the raw h264/h265/mjpeg from
 * gstreamer and packs it into rtp.
 * @param videoCodec the video codec to create the rtp for.
 * @return the gstreamer pipeline part.
 */
static std::string createRtpForVideoCodec(const VideoCodec videoCodec) {
  std::stringstream ss;
  if (videoCodec == VideoCodec::H264) {
    ss << "queue ! ";
    ss << "h264parse config-interval=-1 ! ";
    ss << "rtph264pay mtu=1024 ! ";
  } else if (videoCodec == VideoCodec::H265) {
    ss << "queue ! ";
    ss << "h265parse config-interval=-1 ! ";
    ss << "rtph265pay mtu=1024 ! ";
  } else {
    assert(videoCodec == VideoCodec::MJPEG);
    // mjpeg has no config-interval
    ss << "queue ! ";
    ss << "jpegparse ! ";
    ss << "rtpjpegpay mtu=1024 ! ";
  }
  return ss.str();
}

/**
 * Create the part of the pipeline that takes the rtp from gstreamer and sends
 * it to udp.
 * @param udpOutPort the udp (localhost) port.
 * @return the gstreamer pipeline part
 */
static std::string createOutputUdpLocalhost(const int udpOutPort) {
  return fmt::format(" udpsink host=127.0.0.1 port={} ", udpOutPort);
}

// Needs to match below
static std::string file_suffix_for_video_codec(const VideoCodec videoCodec){
  if(videoCodec==VideoCodec::H264 || videoCodec==VideoCodec::H265){
    return ".mkv";
  }else{
    return ".avi";
  }
}
// Assumes there is a tee command named "t" somewhere in the pipeline right
// after the encoding step, so we can get the raw encoded data out.
// .avi supports h264 and mjpeg, and works even in case the stream would crash
// doesn't support h265 though
// .mp4 is always corrupted on crash
// .mkv supports h264 and h265, but no mjpeg. It is the default in OBS though, so we decided to use .mkv
// for h264 and h265 and .avi for mjpeg
// in case the gst pipeline is not stopped properly
static std::string createRecordingForVideoCodec(const VideoCodec videoCodec,const std::string& out_filename) {
  std::stringstream ss;
  ss << "t. ! queue ! ";
  if (videoCodec == VideoCodec::H264) {
    ss << "h264parse ! ";
  } else if (videoCodec == VideoCodec::H265) {
    ss << "h265parse ! ";
  } else {
    assert(videoCodec == VideoCodec::MJPEG);
    ss << "jpegparse ! ";
  }
  //ss <<"mp4mux ! filesink location="<<out_filename;
  if(videoCodec==VideoCodec::H264 || videoCodec==VideoCodec::H265){
    ss <<"matroskamux ! filesink location="<<out_filename;
  }else{
    ss <<"avimux ! filesink location="<<out_filename;
  }
  return ss.str();
}

static std::string gst_create_caps(const VideoCodec& videoCodec){
  std::stringstream ss;
  if(videoCodec==VideoCodec::H264){
    ss<<"caps=\"application/x-rtp, media=(string)video, encoding-name=(string)H264, payload=(int)96\"";
  }else if(videoCodec==VideoCodec::H265){
    ss<<"caps=\"application/x-rtp, media=(string)video, encoding-name=(string)H265\"";
  }else{
    ss<<"caps=\"application/x-rtp, media=(string)video, encoding-name=(string)mjpeg\"";
  }
  return ss.str();
}
static std::string create_input_custom_udp_rtp_port(const CameraSettings& settings) {
  static constexpr auto input_port=5500;
  static constexpr auto address="127.0.0.1";
  std::stringstream ss;
  ss<<fmt::format("udpsrc address={} port={} {} ! ",address, input_port, gst_create_caps(settings.streamed_video_format.videoCodec));
  if(settings.streamed_video_format.videoCodec==VideoCodec::H264){
    ss<<" rtph264depay ! h264parse ! ";
  }else if(settings.streamed_video_format.videoCodec==VideoCodec::H264){
    ss<<" rtph265depay ! h265parse ! ";
  }else{
    ss<<" rtpjpegpdepay ! jpegparse ! ";
  }
  return ss.str();
}


}  // namespace OHDGstHelper
#endif  // OPENHD_OHDGSTHELPER_H
