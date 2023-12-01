
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
    current.streamed_video_format.videoCodec=VideoCodec::H264;
  }else if(index==1){
    current.streamed_video_format.videoCodec=VideoCodec::H265;
  }else{
    current.streamed_video_format.videoCodec=VideoCodec::MJPEG;
  }
  camera_holder.update_settings(current);
}

int main(int argc, char *argv[]) {
  //
  auto camera_holder=createDummyCamera2();
  update_settings(0,*camera_holder);
  PlatformType platformType{};
  auto stream = std::make_unique<GStreamerStream>(platformType, camera_holder, nullptr);
  stream->start_looping();
  OHDUtil::keep_alive_until_sigterm();
  return 0;
}