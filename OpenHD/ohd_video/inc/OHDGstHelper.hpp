//
// Created by consti10 on 15.05.22.
//

#ifndef OPENHD_OHDGSTHELPER_H
#define OPENHD_OHDGSTHELPER_H

#include <fmt/format.h>
#include <gst/gst.h>

#include <sstream>
#include <string>

#include "openhd-camera.hpp"

/**
 * Helper methods to create parts of gstreamer pipes.
 * Note: Unless a pipeline part is used at the end, all pipelines should end
 * with '! '. This way we avoid syntax errors.
 */
namespace OHDGstHelper {

// some encoders take bits per second instead of kbits per second
static int kbits_to_bits_per_second(int kbit_per_second){
  return kbit_per_second*1024;
}

/**
 * Check if we can find gstreamer at run time, throw a runtime error if not.
 */
static void initGstreamerOrThrow() {
  GError *error = nullptr;
  if (!gst_init_check(nullptr, nullptr, &error)) {
    std::cerr << "gst_init_check() failed: " << error->message << std::endl;
    g_error_free(error);
    throw std::runtime_error("GStreamer initialization failed");
  }
}

// SW encoding is slow, but should work on all platforms (at least for low resolutions/framerate(s) )
// Note that not every sw encoder accepts every type of input format !
// Use 10 for keyframe interval if in doubt.
// Note that the keyframe interval is only for h264 / h265, mjpeg doesn't have a keyframe interval (or rather it is always 1)
static std::string createSwEncoder(const VideoCodec videoCodec,const int bitrateKBits,int keyframe_interval){
  std::stringstream ss;
  if(videoCodec==VideoCodec::H264){
	ss<<"x264enc bitrate="<<bitrateKBits<<" tune=zerolatency key-int-max="<<keyframe_interval<<" ! ";
  }else if(videoCodec==VideoCodec::H265){
	ss<<"x265enc "<<bitrateKBits<<" tune=zerolatency key-int-max="<<keyframe_interval<<" ! ";
  }else{
	assert(videoCodec==VideoCodec::MJPEG);
	//NOTE jpegenc doesn't have a bitrate controll
	ss<<"jpegenc ! ";
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
static std::string createDummyStream(const VideoFormat videoFormat,int bitrateKBits,int keyframe_interval) {
  std::stringstream ss;
  ss << "videotestsrc ! ";
  // h265 cannot do NV12, but I420.
  // x264 and mjpeg can do both NV12 and I420
  // so we use I420 here since every SW encoder can do it.
  ss << fmt::format(
      "video/x-raw, format=I420,width={},height={},framerate={}/1 ! ",
      videoFormat.width, videoFormat.height, videoFormat.framerate);
  // since the primary purpose here is testing, use a fixed low key frame
  // interval.
  ss << createSwEncoder(videoFormat.videoCodec,bitrateKBits,keyframe_interval);
  return ss.str();
}

/**
 * Create a encoded stream for rpicamsrc, which supports h264 only.
 * @param camera_number use -1 to let rpicamsrc decide
 * See https://gstreamer.freedesktop.org/documentation/rpicamsrc/index.html?gi-language=c#GstRpiCamSrcAWBMode for more complicated params
 */
static std::string createRpicamsrcStream(const int camera_number,
                                         const int bitrateKBits,
                                         const VideoFormat videoFormat,
										 int keyframe_interval,
										 int rotation,int awb_mode,int exp_mode) {
  assert(videoFormat.isValid());
  //assert(videoFormat.videoCodec == VideoCodec::H264);
  std::stringstream ss;
  // other than the other ones, rpicamsrc takes bit/s instead of kbit/s
  const int bitrateBitsPerSecond = kbits_to_bits_per_second(bitrateKBits);
  if (camera_number == -1) {
    ss << fmt::format("rpicamsrc bitrate={} preview=0 ",
                      bitrateBitsPerSecond);
  } else {
    ss << fmt::format("rpicamsrc camera-number={} bitrate={} preview=0 ",
                      camera_number, bitrateBitsPerSecond);
  }
  // keyframe-interval   : Interval (in frames) between I frames. -1 = automatic, 0 = single-keyframe
  if(keyframe_interval>= -1 && keyframe_interval < 1000){
	ss << "keyframe-interval="<<keyframe_interval<<" ";
  }else{
	std::cerr<<"Invalid keyframe intervall: "<<keyframe_interval<<"\n";
  }
  if(openhd::needs_horizontal_flip(rotation)){
	ss<<"hflip=1 ";
  }
  if(openhd::needs_vertical_flip(rotation)){
	ss<<"vflip=1 ";
  }
  if(awb_mode!=0){
	ss<<"awb-mode="<<awb_mode<<" ";
  }
  if(exp_mode!=0){
	ss<<"exposure-mode="<<exp_mode<<" ";
  }
  ss<<" ! ";
  if(videoFormat.videoCodec==VideoCodec::H264){
	ss << fmt::format(
		"video/x-h264, profile=constrained-baseline, width={}, height={}, "
		"framerate={}/1, level=3.0 ! ",
		videoFormat.width, videoFormat.height, videoFormat.framerate);
  }else{
	std::cout<<"No h265 / MJPEG encoder on rpi, using SW encode (might result in frame drops/performance issues)\n";
	ss<<fmt::format(
		"video/x-raw, width={}, height={}, framerate={}/1 ! ",
		videoFormat.width, videoFormat.height, videoFormat.framerate);
	ss<<createSwEncoder(videoFormat.videoCodec,bitrateKBits,keyframe_interval);
  }
  return ss.str();
}

/**
 * Create a encoded stream for the jetson, which is fully hardware accelerated
 * for h264,h265 and mjpeg.
 * @param sensor_id sensor id, set to -1 to let nvarguscamerasrc figure it out
 */
static std::string createJetsonStream(const int sensor_id,
                                      const int bitrateKBits,
                                      const VideoFormat videoFormat) {
  assert(videoFormat.videoCodec != VideoCodec::Unknown);
  std::stringstream ss;
  // possible to omit the sensor id, nvarguscamerasrc will then figure out the
  // right sensor id. This only works with one csi camera though.
  if (sensor_id == -1) {
    ss << fmt::format("nvarguscamerasrc do-timestamp=true ! ");
  } else {
    ss << fmt::format("nvarguscamerasrc do-timestamp=true sensor-id={} ! ",
                      sensor_id);
  }
  ss << fmt::format(
      "video/x-raw(memory:NVMM), width={}, height={}, format=NV12, "
      "framerate={}/1 ! ",
      videoFormat.width, videoFormat.height, videoFormat.framerate);
  // https://developer.download.nvidia.com/embedded/L4T/r31_Release_v1.0/Docs/Accelerated_GStreamer_User_Guide.pdf?E_vSS50FKrZaJBjDtnCBmtaY8hWM1QCYlMHtXBqvZ_Jeuw0GXuLNaQwMBWUDABSnWCD-p8ABlBpBpP-kb2ADgWugbW8mgGPxUWJG_C4DWaL1yKjUVMy1AxH1RTaGOW82yFJ549mea--FBPuZUH3TT1MoEd4-sgdrZal5qr1J0McEFeFaVUc&t=eyJscyI6InJlZiIsImxzZCI6IlJFRi1kb2NzLm52aWRpYS5jb21cLyJ9
  // jetson is also bits per second
  const auto bitrateBitsPerSecond =kbits_to_bits_per_second(bitrateKBits);
  if (videoFormat.videoCodec == VideoCodec::H265) {
    ss << fmt::format(
        "nvv4l2h265enc name=vnenc control-rate=1 insert-sps-pps=1 bitrate={} ! ",
        bitrateBitsPerSecond);
  } else if (videoFormat.videoCodec == VideoCodec::H264) {
    ss << fmt::format(
        "nvv4l2h264enc name=nvenc control-rate=1 insert-sps-pps=1 bitrate={} ! ",
        bitrateBitsPerSecond);
  } else {
    ss << fmt::format("nvjpegenc quality=50 ! ");
  }
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
  assert(videoCodec != VideoCodec::Unknown);
  ss << fmt::format("v4l2src name=picturectrl device={} ! ", device_node);
  // rn we omit the set resolution/framerate here and let gstreamer figure it
  // out.
  // TODO: do it better ;)
  std::cout << "Allowing gstreamer to choose UVC format" << std::endl;
  ss << fmt::format("video/x-raw ! ");
  ss << "videoconvert ! ";
  // Add a queue here. With sw we are not low latency anyways.
  ss << "queue ! ";
  assert(videoCodec != VideoCodec::Unknown);
  ss<<createSwEncoder(videoCodec,bitrateKBits,keyframe_interval);
  return ss.str();
}

/**
 * This one is for v4l2src cameras that outputs already encoded video.
 */
static std::string createV4l2SrcAlreadyEncodedStream(
    const std::string &device_node, const VideoFormat videoFormat) {
  std::stringstream ss;
  assert(videoFormat.videoCodec != VideoCodec::Unknown);
  ss << fmt::format("v4l2src name=picturectrl device={} ! ", device_node);
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
      "average-bitrate={} rate-control=1 iframe-period=1000 name=encodectrl "
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
 * @param videoCodec the video codec o create the rtp for.
 * @return the gstreamer pipeline part.
 */
static std::string createRtpForVideoCodec(const VideoCodec videoCodec) {
  std::stringstream ss;
  assert(videoCodec != VideoCodec::Unknown);
  if (videoCodec == VideoCodec::H264) {
    ss << "h264parse config-interval=-1 ! ";
    ss << "rtph264pay mtu=1024 ! ";
  } else if (videoCodec == VideoCodec::H265) {
    ss << "h265parse config-interval=-1 ! ";
    ss << "rtph265pay mtu=1024 ! ";
  } else {
    assert(videoCodec == VideoCodec::MJPEG);
    // mjpeg has no config-interval
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

// Assumes there is a tee command named "t" somewhere in the pipeline right
// after the encoding step, so we can get the raw encoded data out.
static std::string createRecordingForVideoCodec(const VideoCodec videoCodec) {
  assert(videoCodec != VideoCodec::Unknown);
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
  ss << "mp4mux ! filesink location=/tmp/file.mp4";
  return ss.str();
}
}  // namespace OHDGstHelper
#endif  // OPENHD_OHDGSTHELPER_H
