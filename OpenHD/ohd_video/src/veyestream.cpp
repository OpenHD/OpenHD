//
// Created by consti10 on 13.08.22.
//

#include "veyestream.h"
#include "OHDGstHelper.hpp"
#include "openhd-util.hpp"

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
  // Not ideal, needs full path, but veye is hacky anyways
  ss<<"/usr/local/share/veye-raspberrypi/veye_raspivid ";
  const int bitrateBitsPerSecond = OHDGstHelper::kbits_to_bits_per_second(setting.bitrateKBits);
  ss<<"-b "<<bitrateBitsPerSecond<<" ";
  ss<<"-w "<<setting.userSelectedVideoFormat.width<<" ";
  ss<<"-h "<<setting.userSelectedVideoFormat.width<<" ";
  ss<<"-fps "<<setting.userSelectedVideoFormat.framerate<<" ";
  ss<<"--profile constrained-baseline ";
  //ss<<"--level 3 ";
  ss<<"-t 0 -o - ";
  ss<<"| gst-launch-1.0 -v fdsrc ! ";

  ss<<OHDGstHelper::createRtpForVideoCodec(setting.userSelectedVideoFormat.videoCodec);
  ss<<OHDGstHelper::createOutputUdpLocalhost(_video_udp_port);

  pipeline=ss.str();

  std::cout<<"Veye Pipeline:{"<<pipeline<<"}\n";
}

void VEYEStream::restartIfStopped() {
  // unimplemented for veye
}

void VEYEStream::start() {
  std::cout<<"VEYEStream::start() begin\n";
  //const auto res=OHDUtil::run_command(pipeline,{"&"});
  std::cout<<"VEYEStream::start() end\n";
}

void VEYEStream::stop() {
  //OHDUtil::run_command("killall ",{"\"/usr/local/share/veye-raspberrypi/veye_raspivid\""});
}

std::string VEYEStream::createDebug() {
  return "Veye has no debug state";
}

void VEYEStream::restart_async() {
  std::cout<<"VEYEStream::restart_async() begin\n";

  std::cout<<"VEYEStream::restart_async() end\n";
}
