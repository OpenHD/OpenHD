//
// Created by consti10 on 13.08.22.
//

#include "veyestream.h"

VEYEStream::VEYEStream(PlatformType platform, std::shared_ptr<CameraHolder> camera_holder, uint16_t video_udp_port)
	: CameraStream(platform, camera_holder, video_udp_port) {
  std::cout << "VEYEStream::VEYEStream()\n";
  const auto& camera=_camera_holder->get_camera();
  const auto& setting=_camera_holder->get_settings();
  assert(camera.type==CameraType::RaspberryPiVEYE);
  _camera_holder->register_listener([this](){
	// right now, every time the settings for this camera change, we just re-start the whole stream.
	// That is not ideal, since some cameras support changing for example the bitrate or white balance during operation.
	// But wiring that up is not that easy.
	//this->restart_after_new_setting();
	this->restart_async();
  });
  // sanity checks
  assert(setting.userSelectedVideoFormat.isValid());
  std::cout << "VEYEStream::VEYEStream\n";
}
void VEYEStream::setup() {

}
void VEYEStream::restartIfStopped() {

}

void VEYEStream::start() {

}
void VEYEStream::stop() {

}

std::string VEYEStream::createDebug() {
  return "Veye has no debug state";
}

void VEYEStream::restart_async() {

}
