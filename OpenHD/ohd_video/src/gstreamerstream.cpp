
#include <unistd.h>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <regex>

#include <gst/gst.h>

#include "openhd-log.hpp"
#include "OHDGstHelper.hpp"
#include "gstreamerstream.h"

GStreamerStream::GStreamerStream(PlatformType platform,
								 Camera &camera,
								 uint16_t video_udp_port)
	: CameraStream(platform, camera, video_udp_port) {
  std::cout << "GStreamerStream::GStreamerStream()\n";
  // rn the dummy camera doesn't support any custom resolution or framerate
  // since it is sw, 640x48@30 might already be too much on embedded devices anyways.
  if (camera.type == CameraTypeDummy) {
	camera.userSelectedVideoFormat.width = 640;
	camera.userSelectedVideoFormat.height = 480;
	camera.userSelectedVideoFormat.framerate=30;
	std::cout<<"Configured dummy for:"<<m_camera.userSelectedVideoFormat.toString()<<"\n";
  }
}

void GStreamerStream::setup() {
  std::cout << "GStreamerStream::setup()" << std::endl;
  GError *error = nullptr;
  if (!gst_init_check(nullptr, nullptr, &error)) {
	std::cerr << "gst_init_check() failed: " << error->message << std::endl;
	g_error_free(error);
	throw std::runtime_error("GStreamer initialization failed");
  }
  std::cout << "Creating GStreamer pipeline" << std::endl;
  // sanity checks
  if (m_camera.bitrateKBits <= 100 || m_camera.bitrateKBits > (1024 * 1024 * 50)) {
	m_camera.bitrateKBits = 5000;
  }
  assert(m_camera.userSelectedVideoFormat.isValid());
  switch (m_camera.type) {
	case CameraTypeRaspberryPiCSI: {
	  setup_raspberrypi_csi();
	  break;
	}
	case CameraTypeJetsonCSI: {
	  setup_jetson_csi();
	  break;
	}
	case CameraTypeUVC: {
	  setup_usb_uvc();
	  break;
	}
	case CameraTypeUVCH264: {
	  setup_usb_uvch264();
	  break;
	}
	case CameraTypeIP: {
	  setup_ip_camera();
	  break;
	}
	case CameraTypeDummy: {
	  m_pipeline << OHDGstHelper::createDummyStream(m_camera.userSelectedVideoFormat);
	  break;
	}
	case CameraTypeRaspberryPiVEYE:
	case CameraTypeRockchipCSI:
	  std::cerr << "Veye and rockchip are unsupported at the time\n";
	  return;
	case CameraTypeUnknown: {
	  std::cerr << "Unknown camera type" << std::endl;
	  return;
	}
  }
  // After we've written the parts for the different camera implementation(s) we just need to append the rtp part and the udp out
  // add rtp part
  m_pipeline << OHDGstHelper::createRtpForVideoCodec(m_camera.userSelectedVideoFormat.videoCodec);
  // Allows users to fully write a manual pipeline, this must be used carefully.
  if (!m_camera.manual_pipeline.empty()) {
	m_pipeline.str("");
	m_pipeline << m_camera.manual_pipeline;
  }
  // add udp out part
  m_pipeline << OHDGstHelper::createOutputUdpLocalhost(m_video_udp_port);
  std::cout << "Starting pipeline:" << m_pipeline.str() << std::endl;
  gst_pipeline = gst_parse_launch(m_pipeline.str().c_str(), &error);
  if (error) {
	std::cerr << "Failed to create pipeline: " << error->message << std::endl;
	return;
  }
}

void GStreamerStream::setup_raspberrypi_csi() {
  std::cout << "Setting up Raspberry Pi CSI camera" << std::endl;
  m_pipeline<< OHDGstHelper::createRpicamsrcStream(m_camera.bus, m_camera.bitrateKBits, m_camera.userSelectedVideoFormat);
}

void GStreamerStream::setup_jetson_csi() {
  std::cout << "Setting up Jetson CSI camera" << std::endl;
  // Well, i fixed the bug in the detection, with v4l2_open.
  // But still, /dev/video1 can be camera index 0 on jetson.
  // Therefore, for now, we just default to no camera index rn and let nvarguscamerasrc figure out the camera index.
  // This will work as long as there is no more than 1 CSI camera.
  m_pipeline << OHDGstHelper::createJetsonStream(-1, m_camera.bitrateKBits, m_camera.userSelectedVideoFormat);
}

void GStreamerStream::setup_usb_uvc() {
  std::cout << "Setting up usb UVC camera Name:" << m_camera.name << " type:" << m_camera.type << std::endl;
  // First we try and start a hw encoded path, where v4l2src directly provides encoded video buffers
  for (const auto &endpoint: m_camera.endpoints) {
	if (m_camera.userSelectedVideoFormat.videoCodec == VideoCodecH264 && endpoint.support_h264) {
	  std::cerr << "h264" << std::endl;
	  const auto device_node = endpoint.device_node;
	  m_pipeline << OHDGstHelper::createV4l2SrcAlreadyEncodedStream(device_node, m_camera.userSelectedVideoFormat);
	  return;
	}
	if (m_camera.userSelectedVideoFormat.videoCodec == VideoCodecMJPEG && endpoint.support_mjpeg) {
	  std::cerr << "MJPEG" << std::endl;
	  const auto device_node = endpoint.device_node;
	  m_pipeline << OHDGstHelper::createV4l2SrcAlreadyEncodedStream(device_node, m_camera.userSelectedVideoFormat);
	  return;
	}
  }
  // If we land here, we need to do SW encoding, the v4l2src can only do raw video formats like YUV
  for (const auto &endpoint: m_camera.endpoints) {
	std::cout << "empty" << std::endl;
	if (endpoint.support_raw) {
	  const auto device_node = endpoint.device_node;
	  m_pipeline << OHDGstHelper::createV4l2SrcRawAndSwEncodeStream(device_node,
																	m_camera.userSelectedVideoFormat.videoCodec,
																	m_camera.bitrateKBits);
	  return;
	}
  }
  // If we land here, we couldn't create a stream for this camera.
  std::cerr << "Setup USB UVC failed\n";
}

void GStreamerStream::setup_usb_uvch264() {
  std::cout << "Setting up UVC H264 camera" << std::endl;
  const auto endpoint = m_camera.endpoints.front();
  // uvch265 cameras don't seem to exist, codec setting is ignored
  m_pipeline << OHDGstHelper::createUVCH264Stream(endpoint.device_node,
												  m_camera.bitrateKBits,
												  m_camera.userSelectedVideoFormat);
}

void GStreamerStream::setup_ip_camera() {
  std::cout << "Setting up IP camera" << std::endl;
  if (m_camera.url.empty()) {
	m_camera.url = "rtsp://192.168.0.10:554/user=admin&password=&channel=1&stream=0.sdp";
  }
  m_pipeline << OHDGstHelper::createIpCameraStream(m_camera.url);
}

std::string GStreamerStream::createDebug()const {
  std::stringstream ss;
  ss << "GStreamerStream[";
  //ss << "Pipeline:" << m_pipeline.str()<<"\n";
  GstState state;
  GstState pending;
  auto returnValue = gst_element_get_state(gst_pipeline, &state, &pending, 1000000000);
  ss << " Gst state:" << returnValue << "." << state << "." << pending << "." << std::endl;
  ss << "]GStreamerStream";
  return ss.str();
}

void GStreamerStream::start() {
  std::cout << "GStreamerStream::start()" << std::endl;
  gst_element_set_state(gst_pipeline, GST_STATE_PLAYING);
  GstState state;
  GstState pending;
  auto returnValue = gst_element_get_state(gst_pipeline, &state, &pending, 1000000000);
  std::cout << "Gst state:" << returnValue << "." << state << "." << pending << "." << std::endl;
}

void GStreamerStream::stop() {
  std::cout << "GStreamerStream::stop()" << std::endl;
  gst_element_set_state(gst_pipeline, GST_STATE_PAUSED);
}

bool GStreamerStream::supports_bitrate() {
  std::cout << "GStreamerStream::supports_bitrate()" << std::endl;
  return false;
}

void GStreamerStream::set_bitrate(int bitrate) {
  std::cout << "Unmplemented GStreamerStream::set_bitrate(" << bitrate << ")" << std::endl;
}

bool GStreamerStream::supports_cbr() {
  std::cout << "GStreamerStream::supports_cbr()" << std::endl;
  return false;
}

void GStreamerStream::set_cbr(bool enable) {
  std::cout << "Unsupported GStreamerStream::set_cbr(" << enable << ")" << std::endl;
}

VideoFormat GStreamerStream::get_format() {
  std::cout << "GStreamerStream::get_format()" << std::endl;
  return m_camera.userSelectedVideoFormat;
}

void GStreamerStream::set_format(VideoFormat videoFormat) {
  std::cout << "GStreamerStream::set_format(" << videoFormat.toString() << ")" << std::endl;
  m_camera.userSelectedVideoFormat = videoFormat;
}

void GStreamerStream::restartIfStopped() {
  GstState state;
  GstState pending;
  auto returnValue = gst_element_get_state(gst_pipeline, &state, &pending, 1000000000);
  if (returnValue == 0) {
	std::cerr<<"Panic gstreamer pipeline state is not running, restarting camera stream for camera:"<<m_camera.index<<"\n";
	stop();
	sleep(3);
	start();
  }
}






