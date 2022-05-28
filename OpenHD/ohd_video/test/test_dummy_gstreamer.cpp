
#include <iostream>
#include <thread>
#include <chrono>

#include "gstreamerstream.h"

// Independent of OpenHD, start a stream for the dummy camera
// Which rn is implemented in gstreamer.

int main(int argc, char *argv[]) {
  //
  Camera camera;
  camera.type = CameraTypeDummy;
  // Depending on what you selected here, you will have to use the proper main_stream_display_XXX.sh if you want to see the video.
  camera.settings.userSelectedVideoFormat.videoCodec=VideoCodecH264;
  //camera.settings.userSelectedVideoFormat.videoCodec=VideoCodecH265;
  //camera.settings.userSelectedVideoFormat.videoCodec=VideoCodecMJPEG;
  PlatformType platformType;
  uint16_t video_port = OHD_VIDEO_AIR_VIDEO_STREAM_1_UDP;
  auto stream = std::make_unique<GStreamerStream>(platformType, camera, video_port);
  stream->setup();
  stream->start();

  while (true) {
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::cout<<"XOHDVid\n";
	std::cout << stream->createDebug() << "\n";
  }

  std::cerr << "OHDVideo stopped\n";

  return 0;
}