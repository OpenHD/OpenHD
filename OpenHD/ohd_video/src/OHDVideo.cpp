//
// Created by consti10 on 03.05.22.
//
#include "OHDVideo.h"
#include "gstreamerstream.h"
#include "veyestream.h"

OHDVideo::OHDVideo(OHDPlatform platform1,DiscoveredCameraList cameras) :
	platform(platform1) {
  assert(!cameras.empty());
  std::cout << "OHDVideo::OHDVideo()\n";
  std::vector<std::shared_ptr<CameraHolder>> camera_holders;
  for(const auto& camera:cameras){
    camera_holders.emplace_back(std::make_unique<CameraHolder>(camera));
  }
  for (auto &camera: camera_holders) {
	configure(camera);
  }
  std::cout << "OHDVideo::running\n";
}

std::string OHDVideo::createDebug() const {
  // TODO make it much more verbose
  std::stringstream ss;
  ss << "OHDVideo::N camera streams:" << m_camera_streams.size() << "\n";
  for (int i = 0; i < m_camera_streams.size(); i++) {
	const auto &stream = m_camera_streams.at(i);
	ss << "Camera stream:" << i << stream->createDebug() << "\n";
  }
  return ss.str();
}

void OHDVideo::configure(std::shared_ptr<CameraHolder> camera_holder) {
  const auto camera=camera_holder->get_camera();
  std::cout << "Configuring camera: " << camera_type_to_string(camera.type) << std::endl;
  // R.N we use gstreamer only for everything except veye
  // (veye also uses gstreamer, but we do not launch it via gst-launch)
  switch (camera.type) {
	case CameraType::RaspberryPiVEYE:{
	  std::cout << "VEYE stream for Camera index:" << camera.index << "\n";
	  const auto udp_port = camera.index == 0 ? OHD_VIDEO_AIR_VIDEO_STREAM_1_UDP : OHD_VIDEO_AIR_VIDEO_STREAM_2_UDP;
	  auto stream = std::make_shared<VEYEStream>(platform.platform_type, camera_holder, udp_port);
	  stream->setup();
	  stream->start();
	  m_camera_streams.push_back(stream);
	  break;
	}
    case CameraType::RaspberryPiCSI:
    case CameraType::JetsonCSI:
    case CameraType::IP:
    case CameraType::RockchipCSI:
    case CameraType::UVC:
    case CameraType::Dummy: {
      std::cout << "GStreamerStream for Camera index:" << camera.index << "\n";
      const auto udp_port = camera.index == 0 ? OHD_VIDEO_AIR_VIDEO_STREAM_1_UDP : OHD_VIDEO_AIR_VIDEO_STREAM_2_UDP;
      auto stream = std::make_shared<GStreamerStream>(platform.platform_type, camera_holder, udp_port);
      stream->setup();
      stream->start();
      m_camera_streams.push_back(stream);
      break;
    }
	case CameraType::Libcamera: {
	  std::cout << "Camera index:" << camera.index << "\n";
	  const auto udp_port = camera.index == 0 ? OHD_VIDEO_AIR_VIDEO_STREAM_1_UDP : OHD_VIDEO_AIR_VIDEO_STREAM_2_UDP;
	  auto stream = std::make_shared<GStreamerStream>(platform.platform_type, camera_holder, udp_port);
	  stream->setup();
	  stream->start();
	  m_camera_streams.push_back(stream);
	  break;
	}
    default: {
      std::cerr << "Unknown camera type, skipping" << std::endl;
    }
  }
}

void OHDVideo::restartIfStopped() {
  for(auto& stream:m_camera_streams){
	stream->restartIfStopped();
  }
}

std::vector<std::shared_ptr<openhd::ISettingsComponent>> OHDVideo::get_setting_components() {
  std::vector<std::shared_ptr<openhd::ISettingsComponent>> ret;
  for(auto& stream: m_camera_streams){
	ret.push_back(stream->_camera_holder);
  }
  return ret;
}


