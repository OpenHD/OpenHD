//
// Created by consti10 on 13.08.22.
//

#include "veyestream.h"
#include "OHDGstHelper.hpp"
#include "openhd-util.hpp"
#include "veye-helper.h"

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
	this->restart_async();
  });
  // sanity checks
  assert(setting.userSelectedVideoFormat.isValid());
  std::cout << "VEYEStream::VEYEStream\n";
}

void VEYEStream::setup() {
  std::cout<<"VEYEStream::setup() begin\n";
  // kill any already running veye instances
  std::cout<<"kill any already running veye instances\n";
  openhd::veye::kill_all_running_veye_instances();

  _camera_holder->unsafe_get_settings().userSelectedVideoFormat.width=1920;
  _camera_holder->unsafe_get_settings().userSelectedVideoFormat.width=1080;
  _camera_holder->unsafe_get_settings().userSelectedVideoFormat.framerate=30;

  // create the pipeline
  const auto& setting=_camera_holder->get_settings();
  std::stringstream ss;
  // http://wiki.veye.cc/index.php/VEYE-MIPI-290/327_for_Raspberry_Pi
  // Not ideal, needs full path, but veye is hacky anyways
  ss<<"/usr/local/share/veye-raspberrypi/veye_raspivid ";
  const int bitrateBitsPerSecond = OHDGstHelper::kbits_to_bits_per_second(setting.bitrateKBits);
  //const int bitrateBitsPerSecond=4000000;

  ss<<"-b "<<bitrateBitsPerSecond<<" ";
  ss<<"-w "<<setting.userSelectedVideoFormat.width<<" ";
  ss<<"-h "<<setting.userSelectedVideoFormat.height<<" ";
  ss<<"-fps "<<setting.userSelectedVideoFormat.framerate<<" ";
  if(setting.userSelectedVideoFormat.videoCodec==VideoCodec::H264){
	ss<<"--codec H264 ";
	ss<<"--profile baseline ";
  }else if(setting.userSelectedVideoFormat.videoCodec==VideoCodec::MJPEG){
	ss<<"--codec MJPEG ";
  }else{
	std::cerr<<"Veye only supports h264 and MJPEG\n";
  }
   // flush to decrease latency
  ss<<"--flush ";
  // no preview
  ss<<"-n ";
  // forever
  ss<<"-t 0 -o - ";
  //ss<<"| gst-launch-1.0 -v fdsrc ! ";
  ss<<"| gst-launch-1.0 fdsrc ! ";
  ss<<OHDGstHelper::createRtpForVideoCodec(setting.userSelectedVideoFormat.videoCodec);
  ss<<OHDGstHelper::createOutputUdpLocalhost(_video_udp_port);
  pipeline=ss.str();
  // NOTE: We do not execute the pipeline until start() is called
  std::cout<<"Veye Pipeline:{"<<pipeline<<"}\n";
  std::cout<<"VEYEStream::setup() end\n";
}

void VEYEStream::restartIfStopped() {
  // unimplemented for veye
}

void VEYEStream::start() {
  std::cout<<"VEYEStream::start() begin\n";
  // cleanup if existing
  openhd::veye::kill_all_running_veye_instances();
  // don't stream unless enabled
  if(!_camera_holder->get_settings().enable_streaming){
	std::cout<<"Streaming disabled\n";
	return;
  }
  /*assert(_veye_thread== nullptr);
  _veye_thread=std::make_unique<std::thread>( [this]{
	auto res=OHDUtil::run_command_out(pipeline.c_str());
	if(res.has_value()){
	  std::cout<<"Veye thread returned with "<<res.value()<<"\n";
	}
  });*/
  // start streaming (process in the background)
  // run in background and pipe std::cout and std::cerr to file
  OHDUtil::run_command(pipeline,{"&> /tmp/veye.log &"});
  std::cout<<"VEYEStream::start() end\n";
}

void VEYEStream::stop() {
  openhd::veye::kill_all_running_veye_instances();
  /*if(_veye_thread->joinable()){
	std::cout<<"Joining veye thread\n";
	_veye_thread->join();
  }
  _veye_thread=nullptr;*/
}

std::string VEYEStream::createDebug() {
  return "Veye has no debug state";
}

void VEYEStream::restart_async() {
  std::cout<<"VEYEStream::restart_async() begin\n";
  stop();
  setup();
  start();
  std::cout<<"VEYEStream::restart_async() end\n";
}
