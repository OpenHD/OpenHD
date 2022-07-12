
#include <iostream>
#include <thread>
#include <chrono>

#include "gstreamerstream.h"

// Independent of OpenHD, start a stream for the dummy camera
// Which rn is implemented in gstreamer.

static void update_settings(int index,CameraHolder& camera_holder){
  auto current=camera_holder.get_settings();
  // Depending on what you selected here, you will have to use the proper main_stream_display_XXX.sh if you want to see the video.
  if(index==0){
    current.userSelectedVideoFormat.videoCodec=VideoCodec::H264;
  }else if(index==1){
    current.userSelectedVideoFormat.videoCodec=VideoCodec::H265;
  }else{
    current.userSelectedVideoFormat.videoCodec=VideoCodec::MJPEG;
  }
  camera_holder.update_settings(current);
}

int main(int argc, char *argv[]) {
  //
  auto camera_holder=createDummyCamera2();
  PlatformType platformType{};
  uint16_t video_port = OHD_VIDEO_AIR_VIDEO_STREAM_1_UDP;
  auto stream = std::make_unique<GStreamerStream>(platformType, camera_holder, video_port);
  stream->setup();
  stream->start();

  int index=0;

  while (true) {
	std::this_thread::sleep_for(std::chrono::seconds(10));
	std::cout<<"XOHDVid\n";
	std::cout << stream->createDebug() << "\n";
        update_settings(index,*camera_holder);
        index++;
  }

  std::cerr << "OHDVideo stopped\n";

  return 0;
}