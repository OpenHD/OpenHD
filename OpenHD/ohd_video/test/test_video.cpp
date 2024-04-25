
#include <camera_discovery.h>

#include <chrono>
#include <iostream>
#include <thread>

#include "nalu/fragment_helper.h"
#include "ohd_video_air.h"
#include "openhd_bitrate.h"
#include "openhd_link.hpp"
#include "openhd_platform.h"
#include "openhd_profile.h"
#include "openhd_util.h"

//
// Can be used to test / validate a camera implementation.
// R.n prints info about the received frame(s) to stdout.
// (See DummyDebugLink)
//
int main(int argc, char* argv[]) {
  // We need root to read / write camera settings.
  OHDUtil::terminate_if_not_root();
  const auto platform = OHDPlatform::instance();

  auto cameras = OHDVideoAir::discover_cameras();
  openhd::BitrateDebugger bitrate_debugger{"Bitrate", true};

  auto forwarder = openhd::UDPForwarder("127.0.0.1", 5600);
  auto cb = [&forwarder, &bitrate_debugger](
                int stream_index,
                const openhd::FragmentedVideoFrame& fragmented_video_frame) {
    int total_size = 0;
    for (auto& fragemnt : fragmented_video_frame.rtp_fragments) {
      forwarder.forwardPacketViaUDP(fragemnt->data(), fragemnt->size());
      total_size += fragemnt->size();
    }
    if (fragmented_video_frame.dirty_frame) {
      auto fragments =
          make_fragments(fragmented_video_frame.dirty_frame->data(),
                         fragmented_video_frame.dirty_frame->size());
      for (auto& fragment : fragments) {
        forwarder.forwardPacketViaUDP(fragment->data(), fragment->size());
        total_size += fragment->size();
      }
    }
    // openhd::log::get_default()->debug("total size:{}", total_size);
    bitrate_debugger.on_packet(total_size);
  };
  auto debug_link = std::make_shared<DummyDebugLink>();
  debug_link->m_opt_frame_cb = cb;
  OHDVideoAir ohdVideo(cameras, debug_link);
  std::cout << "OHDVideo started\n";
  OHDUtil::keep_alive_until_sigterm();
  std::cerr << "OHDVideo stopped\n";
  return 0;
}
