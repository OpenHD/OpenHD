#include "gstreamerstream.h"

#include <gst/gst.h>
#include <unistd.h>

#include <iostream>
#include <regex>
#include <vector>

#include "AirRecordingFileHelper.hpp"
#include "OHDGstHelper.hpp"
#include "ffmpeg_videosamples.hpp"

GStreamerStream::GStreamerStream(PlatformType platform,std::shared_ptr<CameraHolder> camera_holder,uint16_t video_udp_port)
    : CameraStream(platform, camera_holder, video_udp_port) {
  m_console=openhd::log::create_or_get("v_gststream");
  assert(m_console);
  m_console->debug("GStreamerStream::GStreamerStream()");
  // Since the dummy camera is SW, we generally cannot do more than 640x480@30 anyways.
  // (640x48@30 might already be too much on embedded devices).
  const auto& camera=_camera_holder->get_camera();
  const auto& setting=_camera_holder->get_settings();
  if (camera.type == CameraType::Dummy && (setting.userSelectedVideoFormat.width > 640 ||
      setting.userSelectedVideoFormat.height > 480 || setting.userSelectedVideoFormat.framerate > 30)) {
    m_console->warn("Warning- Dummy camera is done in sw, high resolution/framerate might not work");
    m_console->warn("Configured dummy for:"+setting.userSelectedVideoFormat.toString());
  }
  _camera_holder->register_listener([this](){
    // right now, every time the settings for this camera change, we just re-start the whole stream.
    // That is not ideal, since some cameras support changing for example the bitrate or white balance during operation.
    // But wiring that up is not that easy.
    // We call restart_async() to make sure to not perform heavy operation(s) on the mavlink settings callback, since we need to send the
	// acknowledging response in time. Also, gstreamer and camera(s) are sometimes buggy, so in the worst case gstreamer can become unresponsive
	// and block on the restart operation(s) which would be fatal for telemetry.
	this->restart_async();
  });
  // sanity checks
  if(!check_bitrate_sane(setting.bitrateKBits)){
    // not really needed
  }
  assert(setting.userSelectedVideoFormat.isValid());
  OHDGstHelper::initGstreamerOrThrow();
  m_console->debug("GStreamerStream::GStreamerStream done");
}

void GStreamerStream::setup() {
  m_console->debug("GStreamerStream::setup() begin");
  const auto& camera=_camera_holder->get_camera();
  const auto& setting=_camera_holder->get_settings();
  if(!setting.enable_streaming){
	// When streaming is disabled, we just don't create the pipeline. We fully restart on all changes anyways.
	m_console->info("Streaming disabled");
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
            m_console->error("Veye and rockchip are unsupported at the time");
      return;
    case CameraType::RockchipHDMI: {
      setup_rockchip_hdmi();
      break;
    }
    case CameraType::Unknown: {
      m_console->warn( "Unknown camera type");
      return;
    }
  }
  // quick check,here the pipeline should end with a "! ";
  if(!OHDUtil::endsWith(m_pipeline.str(),"! ")){
	  m_console->error("Probably ill-formatted pipeline:"+m_pipeline.str());
  }
  // for safety we only add the tee command at the right place if recording is enabled.
  if(setting.air_recording==Recording::ENABLED && camera.type != CameraType::RockchipHDMI){
    m_console->info("Air recording active");
    m_pipeline<<"tee name=t ! ";
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
    const auto recording_filename=openhd::video::create_unused_recording_filename(
    OHDGstHelper::file_suffix_for_video_codec(setting.userSelectedVideoFormat.videoCodec));
    {
      std::stringstream ss;
      ss<<"Using ["<<recording_filename<<"] for recording\n";
      m_console->debug(ss.str());
    }
	  m_pipeline<<OHDGstHelper::createRecordingForVideoCodec(setting.userSelectedVideoFormat.videoCodec,recording_filename);
  }
  m_console->debug("Starting pipeline:"+m_pipeline.str());
  // Protect against unwanted use - stop and free the pipeline first
  assert(gst_pipeline== nullptr);
  GError *error = nullptr;
  gst_pipeline = gst_parse_launch(m_pipeline.str().c_str(), &error);
  m_console->debug("GStreamerStream::setup() end");
  if (error) {
	m_console->error( "Failed to create pipeline: {}",error->message);
	return;
  }
}

void GStreamerStream::setup_raspberrypi_csi() {
  m_console->debug("Setting up Raspberry Pi CSI camera");
  // similar to jetson, for now we assume there is only one CSI camera connected.
  const auto& setting=_camera_holder->get_settings();
  m_pipeline<< OHDGstHelper::createRpicamsrcStream(-1, setting.bitrateKBits, setting.userSelectedVideoFormat,setting.keyframe_interval,
												   setting.camera_rotation_degree,
												   setting.awb_mode,setting.exposure_mode);
}

void GStreamerStream::setup_libcamera() {
  m_console->debug("Setting up Raspberry Pi libcamera camera");
  // similar to jetson, for now we assume there is only one CSI camera
  // connected.
  const auto& setting = _camera_holder->get_settings();
  m_pipeline << OHDGstHelper::createLibcamerasrcStream(
      _camera_holder->get_camera().name, setting.bitrateKBits,
      setting.userSelectedVideoFormat,setting.keyframe_interval,
      setting.camera_rotation_degree, setting.awb_mode, setting.exposure_mode);
}

void GStreamerStream::setup_jetson_csi() {
  m_console->debug("Setting up Jetson CSI camera");
  // Well, i fixed the bug in the detection, with v4l2_open.
  // But still, /dev/video1 can be camera index 0 on jetson.
  // Therefore, for now, we just default to no camera index rn and let nvarguscamerasrc figure out the camera index.
  // This will work as long as there is no more than 1 CSI camera.
  const auto& setting=_camera_holder->get_settings();
  m_pipeline << OHDGstHelper::createJetsonStream(-1,setting.bitrateKBits, setting.userSelectedVideoFormat,setting.keyframe_interval);
}

void GStreamerStream::setup_rockchip_hdmi() {
  m_console->debug("Setting up Rockchip HDMI");
  const auto& setting=_camera_holder->get_settings();
  m_pipeline << OHDGstHelper::createRockchipHDMIStream(setting.air_recording==Recording::ENABLED, setting.bitrateKBits, setting.userSelectedVideoFormat, setting.recordingFormat, setting.keyframe_interval);
}

void GStreamerStream::setup_usb_uvc() {
  const auto& camera=_camera_holder->get_camera();
  const auto& setting=_camera_holder->get_settings();
  m_console->debug("Setting up usb UVC camera Name:"+camera.name+" type:"+camera_type_to_string(camera.type));
  // First we try and start a hw encoded path, where v4l2src directly provides encoded video buffers
  for (const auto &endpoint: camera.endpoints) {
	if (setting.userSelectedVideoFormat.videoCodec == VideoCodec::H264 && endpoint.support_h264) {
	  m_console->debug("h264");
	  const auto device_node = endpoint.device_node;
	  m_pipeline << OHDGstHelper::createV4l2SrcAlreadyEncodedStream(device_node, setting.userSelectedVideoFormat);
	  return;
	}
	if (setting.userSelectedVideoFormat.videoCodec == VideoCodec::MJPEG && endpoint.support_mjpeg) {
	  m_console->debug("MJPEG");
	  const auto device_node = endpoint.device_node;
	  m_pipeline << OHDGstHelper::createV4l2SrcAlreadyEncodedStream(device_node, setting.userSelectedVideoFormat);
	  return;
	}
  }
  // If we land here, we need to do SW encoding, the v4l2src can only do raw video formats like YUV
  for (const auto &endpoint: camera.endpoints) {
	m_console->debug("empty");
	if (endpoint.support_raw) {
	  const auto device_node = endpoint.device_node;
          m_pipeline << OHDGstHelper::createV4l2SrcRawAndSwEncodeStream(device_node,
                                                                        setting.userSelectedVideoFormat.videoCodec,
                                                                        setting.bitrateKBits,setting.keyframe_interval);
          return;
	}
  }
  // If we land here, we couldn't create a stream for this camera.
  m_console->error("Setup USB UVC failed");
}

void GStreamerStream::setup_usb_uvch264() {
  m_console->debug("Setting up UVC H264 camera");
  const auto& camera=_camera_holder->get_camera();
  const auto& setting=_camera_holder->get_settings();
  const auto endpoint = camera.endpoints.front();
  // uvch265 cameras don't seem to exist, codec setting is ignored
  m_pipeline << OHDGstHelper::createUVCH264Stream(endpoint.device_node,
                                                  setting.bitrateKBits,
                                                  setting.userSelectedVideoFormat);
}

void GStreamerStream::setup_ip_camera() {
  m_console->debug("Setting up IP camera");
  const auto& camera=_camera_holder->get_camera();
  const auto& setting=_camera_holder->get_settings();
  if (setting.url.empty()) {
	//setting.url = "rtsp://192.168.0.10:554/user=admin&password=&channel=1&stream=0.sdp";
  }
  m_pipeline << OHDGstHelper::createIpCameraStream(setting.url);
}

void GStreamerStream::setup_sw_dummy_camera() {
  m_console->debug("Setting up SW dummy camera");
  const auto& camera=_camera_holder->get_camera();
  const auto& setting=_camera_holder->get_settings();
  m_pipeline << OHDGstHelper::createDummyStream(setting.userSelectedVideoFormat,setting.bitrateKBits,setting.keyframe_interval,setting.mjpeg_quality_percent);
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
  m_console->debug("GStreamerStream::start()");
  if(!gst_pipeline){
	m_console->warn("gst_pipeline==null");
	return;
  }
  gst_element_set_state(gst_pipeline, GST_STATE_PLAYING);
  GstState state;
  GstState pending;
  auto returnValue = gst_element_get_state(gst_pipeline, &state, &pending, 1000000000);
  m_console->debug("Gst state: {} {}.{}",returnValue,state,pending);
}

void GStreamerStream::stop() {
  m_console->debug("GStreamerStream::stop()");
  if(!gst_pipeline){
	m_console->debug("gst_pipeline==null");
	return;
  }
  gst_element_set_state(gst_pipeline, GST_STATE_PAUSED);
}

void GStreamerStream::cleanup_pipe() {
  m_console->debug("GStreamerStream::cleanup_pipe() begin");
  if(!gst_pipeline){
	m_console->debug("gst_pipeline==null");
	return;
  }
  // TODO according to @Alex W we need a EOS signal here to properly shut down the pipeline
  gst_element_set_state (gst_pipeline, GST_STATE_NULL);
  gst_object_unref (gst_pipeline);
  gst_pipeline=nullptr;
  m_console->debug("GStreamerStream::cleanup_pipe() end");
}

void GStreamerStream::restartIfStopped() {
  std::lock_guard<std::mutex> guard(_pipeline_mutex);
  if(!gst_pipeline){
	m_console->debug("gst_pipeline==null");
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
        m_console->warn("Restarting camera, check your parameters / connection");
	stop();
	cleanup_pipe();
	setup();
	start();
	m_console->debug("Restarted");
  }
}

// Restart after a new settings value has been applied
void GStreamerStream::restart_after_new_setting() {
  std::lock_guard<std::mutex> guard(_pipeline_mutex);
  m_console->debug("GStreamerStream::restart_after_new_setting() begin");
  stop();
  // R.N we need to fully re-set the pipeline if any camera setting has changed
  cleanup_pipe();
  setup();
  start();
  m_console->debug("GStreamerStream::restart_after_new_setting() end");
}

void GStreamerStream::restart_async() {
  std::lock_guard<std::mutex> guard(_async_thread_mutex);
  // If there is already an async operation running, we need to wait for it to complete.
  // If the user was to change parameters to quickly, this would be a problem.
  if(_async_thread!= nullptr){
	if(_async_thread->joinable()){
	  m_console->debug("restart_async: waiting for previous operation to finish");
	  _async_thread->join();
	}
	_async_thread=nullptr;
  }
  _async_thread=std::make_unique<std::thread>(&GStreamerStream::restart_after_new_setting,this);
}

