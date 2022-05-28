//
// Created by consti10 on 15.05.22.
//

#ifndef OPENHD_OHDGSTHELPER_H
#define OPENHD_OHDGSTHELPER_H

#include "openhd-camera.hpp"

#include <gst/gst.h>
#include <fmt/format.h>

#include <sstream>
#include <string>

/**
 * Helper methods to create parts of gstreamer pipes.
 * Note: Unless a pipeline part is used at the end, all pipelines should end with '! '.
 * This way we avoid syntax errors.
 */
namespace OHDGstHelper {
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

// a createXXXStream function always ends wth an encoded "h164,h265 or mjpeg stream ! "
// aka after that, onc can add a rtp encoder or similar.
// All these methods also start from zero - aka have a source like videotestsrc, nvarguscamerasr usw in the beginning
// and end with a OpenHD supported video codec (e.g. h264,h265 or mjpeg)
// ------------- crateXXXStream begin -------------
/**
 * Create a encoded dummy stream for the selected video format, that means a stream that takes raw data coming from a videotestsrc
 * and encodes it in either h264, h265 or mjpeg.
 */
static std::string createDummyStream(const VideoFormat videoFormat) {
  std::stringstream ss;
  ss << "videotestsrc ! ";
  // h265 cannot do NV12, but I420.
  // x264 and mjpeg can do both NV12 and I420
  // so we use I420 here since every SW encoder can do it.
  ss << fmt::format("video/x-raw, format=I420,width={},height={},framerate={}/1 ! ",
					videoFormat.width,
					videoFormat.height,
					videoFormat.framerate);
  // since the primary purpose here is testing, use a fixed low key frame intervall.
  if (videoFormat.videoCodec == VideoCodecH264) {
	ss << fmt::format("x264enc bitrate={} tune=zerolatency key-int-max=10 ! ", DEFAULT_BITRATE_KBITS);
  } else if (videoFormat.videoCodec == VideoCodecH265) {
	ss << fmt::format("x265enc bitrate={} tune=zerolatency key-int-max=10 ! ", DEFAULT_BITRATE_KBITS);
  } else {
	ss << "jpegenc !";
  }
  return ss.str();
}

/**
 * Create a encoded stream for rpicamsrc, which supports h264 only.
 * @param camera_number use -1 to let rpicamsrc decide
 */
static std::string createRpicamsrcStream(const int camera_number, const int bitrateKBits, const VideoFormat videoFormat) {
  assert(videoFormat.isValid());
  assert(videoFormat.videoCodec == VideoCodecH264);
  std::stringstream ss;
  // other than the other ones, rpicamsrc takes bit/s instead of kbit/s
  const int bitrateBitsPerSecond=bitrateKBits*1024;
  if(camera_number==-1){
	ss << fmt::format("rpicamsrc bitrate={} preview=0 ! ", bitrateBitsPerSecond);
  }else{
	ss << fmt::format("rpicamsrc camera-number={} bitrate={} preview=0 ! ", camera_number, bitrateBitsPerSecond);
  }
  ss << fmt::format("video/x-h264, profile=constrained-baseline, width={}, height={}, framerate={}/1, level=3.0 ! ",
					videoFormat.width, videoFormat.height, videoFormat.framerate);
  return ss.str();
}

/**
 * Create a encoded stream for the jetson, which is fully hardware accelerated for h264,h265 and mjpeg.
 * @param sensor_id sensor id, set to -1 to let nvarguscamerasrc figure it out
 */
static std::string createJetsonStream(const int sensor_id, const int bitrateKBits, const VideoFormat videoFormat) {
  assert(videoFormat.videoCodec != VideoCodecUnknown);
  std::stringstream ss;
  // possible to omit the sensor id, nvarguscamerasrc will then figure out the right sensor id.
  // This only works with one csi camera though.
  if(sensor_id==-1){
	ss << fmt::format("nvarguscamerasrc do-timestamp=true ! ");
  }else{
	ss << fmt::format("nvarguscamerasrc do-timestamp=true sensor-id={} ! ", sensor_id);
  }
  ss << fmt::format("video/x-raw(memory:NVMM), width={}, height={}, format=NV12, framerate={}/1 ! ",
					videoFormat.width, videoFormat.height, videoFormat.framerate);
  if (videoFormat.videoCodec == VideoCodecH265) {
	ss << fmt::format("nvv4l2h265enc name=vnenc control-rate=1 insert-sps-pps=1 bitrate={} ! ", bitrateKBits);
  } else if (videoFormat.videoCodec == VideoCodecH264) {
	ss << fmt::format("nvv4l2h264enc name=nvenc control-rate=1 insert-sps-pps=1 bitrate={} ! ", bitrateKBits);
  } else {
	ss << fmt::format("nvjpegenc quality=50 ! ");
  }
  return ss.str();
}

/**
 * For V4l2 Cameras that do raw YUV (or RGB) we use a sw encoder.
 * This one has no custom resolution(s) yet.
 */
static std::string createV4l2SrcRawAndSwEncodeStream(const std::string &device_node,
													 const VideoCodec videoCodec,
													 const int bitrateKBits) {
  std::stringstream ss;
  assert(videoCodec != VideoCodecUnknown);
  ss << fmt::format("v4l2src name=picturectrl device={} ! ", device_node);
  // rn we omit the set resolution/framerate here and let gstreamer figure it out.
  // TODO: do it better ;)
  std::cout << "Allowing gstreamer to choose UVC format" << std::endl;
  ss << fmt::format("video/x-raw ! ");
  ss << "videoconvert ! ";
  // Add a queue here. With sw we are not low latency anyways.
  ss << "queue ! ";
  assert(videoCodec != VideoCodecUnknown);
  if (videoCodec == VideoCodecH264) {
	// https://gstreamer.freedesktop.org/documentation/x264/index.html?gi-language=c
	ss << fmt::format("x264enc name=encodectrl bitrate={} tune=zerolatency key-int-max=10 ! ", bitrateKBits);
  } else if (videoCodec == VideoCodecH265) {
	// https://gstreamer.freedesktop.org/documentation/x265/index.html?gi-language=c
	ss << fmt::format("x265enc name=encodectrl bitrate={} ! ", bitrateKBits);
  } else {
	std::cerr << "no sw encoder for MJPEG\n";
  }
  return ss.str();
}

/**
 * This one is for v4l2src cameras that outputs already encoded video.
 */
static std::string createV4l2SrcAlreadyEncodedStream(const std::string &device_node, const VideoFormat videoFormat) {
  std::stringstream ss;
  assert(videoFormat.videoCodec != VideoCodecUnknown);
  ss << fmt::format("v4l2src name=picturectrl device={} ! ", device_node);
  if (videoFormat.videoCodec == VideoCodecH264) {
	ss << fmt::format("video/x-h264, width={}, height={}, framerate={}/1 ! ",
					  videoFormat.width, videoFormat.height, videoFormat.framerate);
  } else if (videoFormat.videoCodec == VideoCodecH265) {
	ss << fmt::format("video/x-h265, width={}, height={}, framerate={}/1 ! ",
					  videoFormat.width, videoFormat.height, videoFormat.framerate);
  } else {
	assert(videoFormat.videoCodec == VideoCodecMJPEG);
	ss << fmt::format("image/jpeg, width={}, height={}, framerate={}/1 ! ",
					  videoFormat.width, videoFormat.height, videoFormat.framerate);
  }
  return ss.str();
}

// These are not tested
static std::string createUVCH264Stream(const std::string &device_node,
									   const int bitrateKBits,
									   const VideoFormat videoFormat) {
  assert(videoFormat.videoCodec == VideoCodecH264);
  // https://gstreamer.freedesktop.org/documentation/uvch264/uvch264src.html?gi-language=c#uvch264src:average-bitrate
  // bitrate in bits per second
  const int bitrateBitsPerSecond=bitrateKBits*1024;
  std::stringstream ss;
  ss << fmt::format(
	  "uvch264src device={} peak-bitrate={} initial-bitrate={} average-bitrate={} rate-control=1 iframe-period=1000 name=encodectrl auto-start=true encodectrl.vidsrc ! ",
	  device_node,
	  bitrateBitsPerSecond,
	  bitrateBitsPerSecond,
	  bitrateBitsPerSecond);
  ss << fmt::format("video/x-h264,width={}, height={}, framerate={}/1 ! ",
					videoFormat.width, videoFormat.height, videoFormat.framerate);
  return ss.str();
}
static std::string createIpCameraStream(const std::string &url) {
  std::stringstream ss;
  // none of the other params are used at the moment, we would need to set them with ONVIF or a per-camera API of some sort,
  // however most people seem to set them in advance with the proprietary app that came with the camera anyway
  ss << fmt::format("rtspsrc location=\"{}\" latency=0 ! ", url);
  return ss.str();
}
// ------------- crateXXXStream end  -------------

/**
* Create the part of the pipeline that takes the raw h264/h265/mjpeg from gstreamer and packs it into rtp.
* @param videoCodec the video codec o create the rtp for.
* @return the gstreamer pipeline part.
*/
static std::string createRtpForVideoCodec(const VideoCodec videoCodec) {
  std::stringstream ss;
  assert(videoCodec != VideoCodecUnknown);
  if (videoCodec == VideoCodecH264) {
	ss << "h264parse config-interval=-1 ! ";
	ss << "rtph264pay mtu=1024 ! ";
  } else if (videoCodec == VideoCodecH265) {
	ss << "h265parse config-interval=-1 ! ";
	ss << "rtph265pay mtu=1024 ! ";
  } else {
	assert(videoCodec == VideoCodecMJPEG);
	// mjpeg has no config-interval
	ss << "jpegparse ! ";
	ss << "rtpjpegpay mtu=1024 ! ";
  }
  return ss.str();
}

/**
* Create the part of the pipeline that takes the rtp from gstreamer and sends it to udp.
* @param udpOutPort the udp (localhost) port.
* @return the gstreamer pipeline part
*/
static std::string createOutputUdpLocalhost(const int udpOutPort) {
  return fmt::format(" udpsink host=127.0.0.1 port={} ", udpOutPort);
}
}
#endif //OPENHD_OHDGSTHELPER_H
