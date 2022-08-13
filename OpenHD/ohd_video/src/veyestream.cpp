//
// Created by consti10 on 13.08.22.
//

#include "veyestream.h"
#include "OHDGstHelper.hpp"

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
  const auto& setting=_camera_holder->get_settings();
  std::stringstream ss;
  // http://wiki.veye.cc/index.php/VEYE-MIPI-290/327_for_Raspberry_Pi
  ss<<"veye_raspivid ";
  const int bitrateBitsPerSecond = OHDGstHelper::kbits_to_bits_per_second(setting.bitrateKBits);
  ss<<"-b "<<bitrateBitsPerSecond<<" ";
  ss<<"-w "<<setting.userSelectedVideoFormat.width<<" ";
  ss<<"-h "<<setting.userSelectedVideoFormat.width<<" ";
  ss<<"-fps "<<setting.userSelectedVideoFormat.framerate<<" ";
  ss<<"-t 0 -o - ";
  ss<<"| gst-launch-1.0 -v fdsrc ! ";

  ss<<OHDGstHelper::createRtpForVideoCodec(setting.userSelectedVideoFormat.videoCodec);
  ss<<OHDGstHelper::createOutputUdpLocalhost(_video_udp_port);

  std::cout<<"Veye Pipeline:{"<<ss.str()<<"}\n";
}

void VEYEStream::restartIfStopped() {
  // unimplemented for veye
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
