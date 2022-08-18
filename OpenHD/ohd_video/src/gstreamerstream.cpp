#include "gstreamerstream.h"

#include <unistd.h>
#include <iostream>
#include <vector>
#include <regex>

#include <gst/gst.h>

#include "openhd-log.hpp"
#include "OHDGstHelper.hpp"
#include "ffmpeg_videosamples.hpp"

GStreamerStream::GStreamerStream(PlatformType platform,std::shared_ptr<CameraHolder> camera_holder,uint16_t video_udp_port)
    : CameraStream(platform, camera_holder, video_udp_port) {
  std::cout << "GStreamerStream::GStreamerStream()\n";
  // Since the dummy camera is SW, we generally cannot do more than 640x480@30 anyways.
  // (640x48@30 might already be too much on embedded devices).
  const auto& camera=_camera_holder->get_camera();
  const auto& setting=_camera_holder->get_settings();
  if (camera.type == CameraType::Dummy && setting.userSelectedVideoFormat.width > 640 ||
      setting.userSelectedVideoFormat.height > 480 || setting.userSelectedVideoFormat.framerate > 30) {
    std::cout<<"Warning- Dummy camera is done in sw, high resolution/framerate might not work\n";
    std::cout<<"Configured dummy for:" << setting.userSelectedVideoFormat.toString() << "\n";
  }
  _camera_holder->register_listener([this](){
    // right now, every time the settings for this camera change, we just re-start the whole stream.
    // That is not ideal, since some cameras support changing for example the bitrate or white balance during operation.
    // But wiring that up is not that easy.
    //this->restart_after_new_setting();
	this->restart_async();
  });
  // sanity checks
  if(!check_bitrate_sane(setting.bitrateKBits)){
    //std::cerr << "manually fixing insane camera bitrate" << m_camera.settings.bitrateKBits << "\n";
    //m_camera.settings.bitrateKBits=DEFAULT_BITRATE_KBITS;
  }
  assert(setting.userSelectedVideoFormat.isValid());
  OHDGstHelper::initGstreamerOrThrow();
  std::cout << "GStreamerStream::GStreamerStream done\n";
}

void GStreamerStream::setup() {
  std::cout << "GStreamerStream::setup() begin\n";
  const auto& camera=_camera_holder->get_camera();
  const auto& setting=_camera_holder->get_settings();
  if(!setting.enable_streaming){
	// When streaming is disabled, we just don't create the pipeline. We fully restart on all changes anyways.
	std::cout<<"Streaming disabled\n";
	return;
  }
  m_pipeline.str("");
  m_pipeline.clear();
  switch (camera.type) {
	case CameraType::RaspberryPiCSI: {
	  setup_raspberrypi_csi();
	  break;
	}
    case CameraType::Libcamera: {
      setup_libcamera();
      break;
    }
    case CameraType::JetsonCSI: {
	  setup_jetson_csi();
	  break;
	}
	case CameraType::UVC: {
	  setup_usb_uvc();
	  break;
	}
	case CameraType::UVCH264: {
	  setup_usb_uvch264();
	  break;
	}
	case CameraType::IP: {
	  setup_ip_camera();
	  break;
	}
	case CameraType::Dummy: {
	  setup_sw_dummy_camera();
	  break;
	}
	case CameraType::RaspberryPiVEYE:
	case CameraType::RockchipCSI:
	  std::cerr << "Veye and rockchip are unsupported at the time\n";
	  return;
	case CameraType::Unknown: {
	  std::cerr << "Unknown camera type" << std::endl;
	  return;
	}
  }
  // quick check,here the pipeline should end with a "! ";
  if(!OHDUtil::endsWith(m_pipeline.str(),"! ")){
	std::cerr<<"Probably ill-formatted pipeline:"<<m_pipeline.str()<<"\n";
  }
  // TODO: ground recording is not working yet, since we cannot properly close the file at the time.
  //setting.enableAirRecordingToFile = false;
  // for lower latency we only add the tee command at the right place if recording is enabled.
  if(setting.air_recording==Recording::ENABLED){
	std::cout<<"Air recording active\n";
	//m_pipeline<<"tee name=t ! queue ! ";
  }
  // After we've written the parts for the different camera implementation(s) we just need to append the rtp part and the udp out
  // add rtp part
  m_pipeline << OHDGstHelper::createRtpForVideoCodec(setting.userSelectedVideoFormat.videoCodec);
  // Allows users to fully write a manual pipeline, this must be used carefully.
  /*if (!m_camera.settings.manual_pipeline.empty()) {
	m_pipeline.str("");
	m_pipeline << m_camera.settings.manual_pipeline;
  }*/
  // add udp out part
  m_pipeline << OHDGstHelper::createOutputUdpLocalhost(_video_udp_port);
  if(setting.air_recording==Recording::ENABLED){
	//m_pipeline<<OHDGstHelper::createRecordingForVideoCodec(setting.userSelectedVideoFormat.videoCodec);
  }
  std::cout << "Starting pipeline:" << m_pipeline.str() << std::endl;
  // Protect against unwanted use - stop and free the pipeline first
  assert(gst_pipeline== nullptr);
  GError *error = nullptr;
  gst_pipeline = gst_parse_launch(m_pipeline.str().c_str(), &error);
  std::cout << "GStreamerStream::setup() end\n";
  if (error) {
	std::cerr << "Failed to create pipeline: " << error->message << std::endl;
	return;
  }
}

void GStreamerStream::setup_raspberrypi_csi() {
  std::cout << "Setting up Raspberry Pi CSI camera" << std::endl;
  // similar to jetson, for now we assume there is only one CSI camera connected.
  const auto& setting=_camera_holder->get_settings();
  m_pipeline<< OHDGstHelper::createRpicamsrcStream(-1, setting.bitrateKBits, setting.userSelectedVideoFormat,setting.keyframe_interval,
												   setting.camera_rotation_degree,
												   setting.awb_mode,setting.exposure_mode);
}

void GStreamerStream::setup_libcamera() {
  std::cout << "Setting up Raspberry Pi CSI camera" << std::endl;
  // similar to jetson, for now we assume there is only one CSI camera
  // connected.
  const auto& setting = _camera_holder->get_settings();
  m_pipeline << OHDGstHelper::createLibcamerasrcStream(
      _camera_holder->get_camera().name, setting.bitrateKBits,
      setting.userSelectedVideoFormat,setting.keyframe_interval,
      setting.camera_rotation_degree, setting.awb_mode, setting.exposure_mode);
}

void GStreamerStream::setup_jetson_csi() {
  std::cout << "Setting up Jetson CSI camera" << std::endl;
  // Well, i fixed the bug in the detection, with v4l2_open.
  // But still, /dev/video1 can be camera index 0 on jetson.
  // Therefore, for now, we just default to no camera index rn and let nvarguscamerasrc figure out the camera index.
  // This will work as long as there is no more than 1 CSI camera.
  const auto& setting=_camera_holder->get_settings();
  m_pipeline << OHDGstHelper::createJetsonStream(-1,setting.bitrateKBits, setting.userSelectedVideoFormat,setting.keyframe_interval);
}

void GStreamerStream::setup_usb_uvc() {
  const auto& camera=_camera_holder->get_camera();
  const auto& setting=_camera_holder->get_settings();
  std::cout << "Setting up usb UVC camera Name:" << camera.name << " type:" << camera_type_to_string(camera.type) << std::endl;
  // First we try and start a hw encoded path, where v4l2src directly provides encoded video buffers
  for (const auto &endpoint: camera.endpoints) {
	if (setting.userSelectedVideoFormat.videoCodec == VideoCodec::H264 && endpoint.support_h264) {
	  std::cerr << "h264" << std::endl;
	  const auto device_node = endpoint.device_node;
	  m_pipeline << OHDGstHelper::createV4l2SrcAlreadyEncodedStream(device_node, setting.userSelectedVideoFormat);
	  return;
	}
	if (setting.userSelectedVideoFormat.videoCodec == VideoCodec::MJPEG && endpoint.support_mjpeg) {
	  std::cerr << "MJPEG" << std::endl;
	  const auto device_node = endpoint.device_node;
	  m_pipeline << OHDGstHelper::createV4l2SrcAlreadyEncodedStream(device_node, setting.userSelectedVideoFormat);
	  return;
	}
  }
  // If we land here, we need to do SW encoding, the v4l2src can only do raw video formats like YUV
  for (const auto &endpoint: camera.endpoints) {
	std::cout << "empty" << std::endl;
	if (endpoint.support_raw) {
	  const auto device_node = endpoint.device_node;
          m_pipeline << OHDGstHelper::createV4l2SrcRawAndSwEncodeStream(device_node,
                                                                        setting.userSelectedVideoFormat.videoCodec,
                                                                        setting.bitrateKBits,setting.keyframe_interval);
          return;
	}
  }
  // If we land here, we couldn't create a stream for this camera.
  std::cerr << "Setup USB UVC failed\n";
}

void GStreamerStream::setup_usb_uvch264() {
  std::cout << "Setting up UVC H264 camera" << std::endl;
  const auto& camera=_camera_holder->get_camera();
  const auto& setting=_camera_holder->get_settings();
  const auto endpoint = camera.endpoints.front();
  // uvch265 cameras don't seem to exist, codec setting is ignored
  m_pipeline << OHDGstHelper::createUVCH264Stream(endpoint.device_node,
                                                  setting.bitrateKBits,
                                                  setting.userSelectedVideoFormat);
}

void GStreamerStream::setup_ip_camera() {
  std::cout << "Setting up IP camera" << std::endl;
  const auto& camera=_camera_holder->get_camera();
  const auto& setting=_camera_holder->get_settings();
  if (setting.url.empty()) {
	//setting.url = "rtsp://192.168.0.10:554/user=admin&password=&channel=1&stream=0.sdp";
  }
  m_pipeline << OHDGstHelper::createIpCameraStream(setting.url);
}

void GStreamerStream::setup_sw_dummy_camera() {
  std::cout << "Setting up SW dummy camera\n";
  const auto& camera=_camera_holder->get_camera();
  const auto& setting=_camera_holder->get_settings();
  m_pipeline << OHDGstHelper::createDummyStream(setting.userSelectedVideoFormat,setting.bitrateKBits,setting.keyframe_interval);
}

std::string GStreamerStream::createDebug(){
  std::unique_lock<std::mutex> lock(_pipeline_mutex, std::try_to_lock);
  if(!lock.owns_lock()){
	// We can just discard statistics data during a re-start
	return "GStreamerStream::No debug during restart\n";
  }
  if(!_camera_holder->get_settings().enable_streaming){
	std::stringstream ss;
	ss << "GStreamerStream for camera:"<<_camera_holder->get_camera().debugName()<<" disabled";
	return ss.str();
  }
  std::stringstream ss;
  GstState state;
  GstState pending;
  auto returnValue = gst_element_get_state(gst_pipeline, &state, &pending, 1000000000);
  ss << "GStreamerStream for camera:"<<_camera_holder->get_camera().debugName()<<" State:"<< returnValue << "." << state << "." << pending << ".";
  return ss.str();
}

void GStreamerStream::start() {
  std::cout << "GStreamerStream::start()\n";
  if(!gst_pipeline){
	std::cout<<"gst_pipeline==null\n";
	return;
  }
  gst_element_set_state(gst_pipeline, GST_STATE_PLAYING);
  GstState state;
  GstState pending;
  auto returnValue = gst_element_get_state(gst_pipeline, &state, &pending, 1000000000);
  std::cout << "Gst state:" << returnValue << "." << state << "." << pending << "." << std::endl;
}

void GStreamerStream::stop() {
  std::cout << "GStreamerStream::stop()\n";
  if(!gst_pipeline){
	std::cout<<"gst_pipeline==null\n";
	return;
  }
  gst_element_set_state(gst_pipeline, GST_STATE_PAUSED);
}

void GStreamerStream::cleanup_pipe() {
  std::cout<<"GStreamerStream::cleanup_pipe() begin\n";
  if(!gst_pipeline){
	std::cout<<"gst_pipeline==null\n";
	return;
  }
  gst_element_set_state (gst_pipeline, GST_STATE_NULL);
  gst_object_unref (gst_pipeline);
  gst_pipeline=nullptr;
  std::cout<<"GStreamerStream::cleanup_pipe() end\n";
}

void GStreamerStream::restartIfStopped() {
  std::lock_guard<std::mutex> guard(_pipeline_mutex);
  if(!gst_pipeline){
	std::cout<<"gst_pipeline==null\n";
	return;
  }
  GstState state;
  GstState pending;
  auto returnValue = gst_element_get_state(gst_pipeline, &state, &pending, 1000000000); // timeout in ns
  if (returnValue == 0) {
	std::stringstream message;
	message<<"Panic gstreamer pipeline state is not running, restarting camera stream for camera:"<<_camera_holder->get_camera().name<<"\n";
	// We fully restart the whole pipeline, since some issues might not be fixable by just setting paused
	// Log such that it shows up in QOpenHD
	LOGE<<"Restarting camera, check your parameters / connection";
	stop();
	cleanup_pipe();
	setup();
	start();
	std::cerr<<"Restarted\n";
  }
}

// Restart after a new settings value has been applied
void GStreamerStream::restart_after_new_setting() {
  std::lock_guard<std::mutex> guard(_pipeline_mutex);
  std::cout<<"GStreamerStream::restart_after_new_setting() begin\n";
  stop();
  // R.N we need to fully re-set the pipeline if any camera setting has changed
  cleanup_pipe();
  setup();
  start();
  std::cout<<"GStreamerStream::restart_after_new_setting() end\n";
}

void GStreamerStream::restart_async() {
  std::lock_guard<std::mutex> guard(_async_thread_mutex);
  // If there is already an async operation running, we need to wait for it to complete.
  // If the user was to change parameters to quickly, this would be a problem.
  if(_async_thread!= nullptr){
	if(_async_thread->joinable()){
	  _async_thread->join();
	}
	_async_thread=nullptr;
  }
  _async_thread=std::make_unique<std::thread>(&GStreamerStream::restart_after_new_setting,this);
}

