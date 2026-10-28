#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <thread>

#include "wb_radio_controls.h"
#include "wifi_test.h"
using namespace openhd::wifi_test;
static void require(bool okay, const char* message) {
  if (!okay) throw std::runtime_error(message);
}
int main() {
  try {
    require(!describe_usb_radio(0x1234, 0x5678, 0x04),
            "Unrelated USB device admitted as a radio");
    require(!describe_usb_radio(0x0bda, 0x5411, 0),
            "Unknown Realtek hub admitted as a radio");
    require(!describe_usb_radio(0x0bda, 0x8812, 0),
            "Unidentified shared Realtek PID admitted as a radio");
    require(!describe_usb_radio(0x0b05, 0x17d2, 0xff),
            "Retail USB ID bypassed unknown silicon rejection");
    const auto asus = describe_usb_radio(0x0b05, 0x17d2, 0x04);
    require(asus && asus->at("model") == "ASUS USB-AC56" &&
                asus->at("chipset") == "RTL8812AU" &&
                asus->at("label").get<std::string>().find("ASUS USB-AC56") !=
                    std::string::npos,
            "ASUS USB-AC56 chipset/model identity lost");
    const auto generic = describe_usb_radio(0x0bda, 0x8812, 0x04);
    require(generic && generic->at("model") == "" &&
                generic->at("chip") == "RTL8812A",
            "Generic USB ID was assigned an unverified retail model");
    const auto setup = describe_usb_setup_candidate(0x0b05, 0x17d2);
    require(setup && setup->at("model") == "ASUS USB-AC56" &&
                !setup->at("ready").get<bool>() &&
                setup->at("needs_driver_setup").get<bool>(),
            "Known ASUS awaiting driver setup was lost or marked TX-ready");
    require(!describe_usb_setup_candidate(0x0bda, 0x5411) &&
                !describe_usb_setup_candidate(0x1234, 0x5678) &&
                !describe_usb_setup_candidate(0x0bda, 0x1a2b),
            "Non-radio USB device offered for driver replacement");
    Settings s;
    s.device = "devourer-usb-1-2";
    require(settings_from_json(settings_json(s)) == s,
            "Settings round-trip failed");
    for (const auto& c : channels()) {
      s.frequency = c.at("mhz");
      validate(s);
    }
    s.frequency = 5745;
    for (int m = 0; m <= 31; ++m) {
      s.mcs = m;
      validate(s);
    }
    for (int w : {5, 80, 160}) {
      s.width = w;
      bool rejected = false;
      try {
        validate(s);
      } catch (const std::exception&) {
        rejected = true;
      }
      require(rejected, "Unavailable OpenHD width accepted");
    }
    s.width = 20;
    s.mcs = 32;
    bool rejected = false;
    try {
      validate(s);
    } catch (const std::exception&) {
      rejected = true;
    }
    require(rejected, "Invalid MCS accepted");
    require(openhd::devourer_power_offset_qdb(20) == -64 &&
                openhd::devourer_power_offset_qdb(100) == 0,
            "Production power curve changed");
    require(
        !openhd::is_valid_power_level(155) && openhd::is_valid_power_level(150),
        "Power validation failed");
    const auto frames = sample_video_frames();
    require(frames.size() == 30,
            "Sample must contain 30 complete access units");
    require(sample_video_sha256().size() == 64, "Sample hash unavailable");
    s.mcs = 2;
    PayloadSource a(s), b(s);
    const auto first = a.next(), same = b.next();
    require(first.size() == 1 && *first.front() == *same.front(),
            "Garbage payload is not reproducible");
    require(*a.next().front() != *first.front(),
            "Garbage source repeated a packet");
    s.video = true;
    s.packet_bytes = 600;
    PayloadSource video(s);
    int sequence = -1;
    for (int frame = 0; frame < 31; ++frame) {
      const auto fragments = video.next();
      require(!fragments.empty(), "Empty sample video frame");
      for (const auto& f : fragments) {
        require(f->size() <= 600 && f->size() >= 12, "RTP MTU violated");
        require((*f)[0] >> 6 == 2 && ((*f)[1] & 127) == 96,
                "Invalid H264 RTP header");
        const int next = ((*f)[2] << 8) | (*f)[3];
        if (sequence >= 0)
          require(next == ((sequence + 1) & 65535),
                  "RTP sequence discontinuity");
        sequence = next;
      }
      require((*fragments.back())[1] & 128,
              "Access unit missing final RTP marker");
    }
    Controller controller;
    s.device.clear();
    controller.start(s, "unused");
    for (int i = 0; i < 100 && controller.snapshot().at("state") != "error";
         ++i)
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    require(controller.snapshot().at("state") == "error",
            "Invalid request did not report an error asynchronously");
    controller.stop();
    for (int i = 0; i < 100 && controller.snapshot().at("state") != "idle"; ++i)
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    s.device = "not-a-real-radio";
    const auto output =
        std::filesystem::temp_directory_path() /
        ("openhd-wifi-contract-" +
         std::to_string(
             std::chrono::steady_clock::now().time_since_epoch().count()));
    controller.start(s, output);
    for (int i = 0; i < 1000 && controller.snapshot().at("state") != "error";
         ++i)
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    require(controller.snapshot().at("state") == "error",
            "Missing radio did not fail cleanly");
    std::ifstream journal(output / "session.jsonl");
    std::string line;
    bool requested = false, error = false, stopped = false;
    int64_t previous = -1;
    while (std::getline(journal, line)) {
      const auto row = Json::parse(line);
      const auto type = row.at("type");
      const int64_t now = row.at("elapsed_us");
      require(now >= previous, "Session clock moved backwards");
      previous = now;
      requested |= type == "start_requested";
      error |= type == "error";
      stopped |= type == "stop";
      require(type != "applied",
              "Failed radio was recorded as successfully applied");
      require(row.at("settings") == settings_json(s),
              "Requested settings lost from session");
    }
    require(requested && error && stopped,
            "Failure session is missing audit events");
    require(std::filesystem::file_size(output / "samples.csv") > 0,
            "CSV header was not saved");
    journal.close();
    std::filesystem::remove(output / "session.jsonl");
    std::filesystem::remove(output / "samples.csv");
    std::filesystem::remove(output);
    std::cout << "Shared limits, settings, video asset and controller error "
                 "lifecycle passed\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
}
