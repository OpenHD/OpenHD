//
// Created by consti10 on 03.05.22.
//

#include "gstreamerstream.h"
#include "libcamerastream.h"
#include "openhd-settings.hpp"

#include "OHDVideo.h"

#include <utility>

OHDVideo::OHDVideo(const OHDPlatform &platform, const OHDProfile &profile) : platform(platform), profile(profile) {
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

void OHDVideo::process_manifest() {
  m_cameras = cameras_from_manifest();
  if (m_cameras.empty()) {
	std::cerr << "Started as air but no camera(s) have been found from json.Adding dummy camera\n";
	// If there is no camera, but we are running as air, create and start a dummy camera:
	Camera camera{};
	camera.type = CameraTypeDummy;
	m_cameras.push_back(camera);
  }
}

void OHDVideo::setup() {
  std::cout << "OHDVideo::setup()" << std::endl;
  process_manifest();
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
	  auto stream = std::make_unique<GStreamerStream>(platform.platform_type, camera, udp_port);
	  stream->setup();
	  stream->start();
	  m_camera_streams.push_back(std::move(stream));
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


