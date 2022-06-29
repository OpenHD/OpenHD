//
// Created by consti10 on 03.05.22.
//

#include "gstreamerstream.h"
#include "libcamerastream.h"
#include "openhd-settings.hpp"
#include "DCameras.h"

#include "OHDVideo.h"

#include <utility>

OHDVideo::OHDVideo(const OHDPlatform &platform, const OHDProfile &profile,DiscoveredCameraList cameras) :
	platform(platform), profile(profile),m_cameras(cameras) {
  assert(("This module must only run on the air pi !", profile.is_air == true));
  std::cout << "OHDVideo::OHDVideo()\n";
  try {
	setup();
  } catch (std::exception &ex) {
	std::cerr << "Error: " << ex.what() << std::endl;
	exit(1);
  } catch (...) {
	std::cerr << "Unknown exception occurred" << std::endl;
	exit(1);
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

void OHDVideo::setup() {
  std::cout << "OHDVideo::setup()" << std::endl;
  // Consti10 sanity checks
  for (auto &camera: m_cameras) {
	// check to see if we need to set a default bitrate.
	if (!check_bitrate_sane(camera.settings.bitrateKBits)) {
	  camera.settings.bitrateKBits = DEFAULT_BITRATE_KBITS;
	}
  }
  for (auto &camera: m_cameras) {
	configure(camera);
  }
}

void OHDVideo::configure(Camera &camera) {
  std::cerr << "Configuring camera: " << camera_type_to_string(camera.type) << std::endl;
  // these are all using gstreamer at the moment, but that may not be the case forever
  switch (camera.type) {
	case CameraTypeRaspberryPiCSI:
	case CameraTypeRaspberryPiVEYE:
	case CameraTypeJetsonCSI:
	case CameraTypeIP:
	case CameraTypeRockchipCSI:
	case CameraTypeUVC:
	case CameraTypeDummy: {
	  std::cout << "Camera index:" << camera.index << "\n";
	  const auto udp_port = camera.index == 0 ? OHD_VIDEO_AIR_VIDEO_STREAM_1_UDP : OHD_VIDEO_AIR_VIDEO_STREAM_2_UDP;
	  auto stream = std::make_shared<GStreamerStream>(platform.platform_type, camera, udp_port);
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

bool OHDVideo::set_video_format(int stream_idx, const VideoFormat video_format) {
  auto stream= get_stream_by_index(stream_idx);
  if(!stream)return false;
  stream->set_format(video_format);
  return true;
}
std::shared_ptr<CameraStream> OHDVideo::get_stream_by_index(int idx) {
  if(idx<m_cameras.size()){
    return m_camera_streams[idx];
  }
  return nullptr;
}


