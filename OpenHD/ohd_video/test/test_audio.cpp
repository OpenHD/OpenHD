

#include <iostream>
#include <thread>

#include "gstaudiostream.h"
#include "openhd_bitrate.h"
#include "openhd_udp.h"
#include "openhd_util.h"

//
// Can be used to test / validate a camera implementation.
// R.n prints info about the received frame(s) to stdout.
// (See DummyDebugLink)
//
int main(int argc, char* argv[]) {
  // We need root to read / write camera settings.
  OHDUtil::terminate_if_not_root();
  openhd::BitrateDebugger bitrate_debugger{"Bitrate", true};
  auto forwarder = openhd::UDPForwarder("127.0.0.1", 5610);
  auto cb = [&forwarder,
             &bitrate_debugger](const openhd::AudioPacket& audioPacket) {
    forwarder.forwardPacketViaUDP(audioPacket.data->data(),
                                  audioPacket.data->size());
    // openhd::log::get_default()->debug("total size:{}",
    //                                   audioPacket.data->size());
    bitrate_debugger.on_packet(audioPacket.data->size());
  };
  auto audiostream = std::make_unique<GstAudioStream>();
  audiostream->set_link_cb(cb);
  audiostream->start_looping();
  std::cout << "OHDVideo started\n";
  OHDUtil::keep_alive_until_sigterm();
  std::cerr << "OHDVideo stopped\n";
  return 0;
}
