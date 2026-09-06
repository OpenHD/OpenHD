#include "nexmon_scout.h"

#include <pcap/pcap.h>
#include <poll.h>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>

#ifdef OHD_ENABLE_DEVOURER
#include "chanmig/SurveyRecord.h"
#endif

namespace openhd {
namespace {
constexpr const char* helper =
    "timeout 45 /usr/bin/python3 /usr/local/libexec/openhd-nexmon-scout ";
uint16_t le16(const uint8_t* p) { return p[0] | (uint16_t(p[1]) << 8); }
}

bool NexmonScout::installed() {
  // ARM images also run on other boards. Bundle presence alone must not
  // divert their scan commands into the BCM43455-specific backend.
  std::ifstream model_file("/proc/device-tree/model");
  std::string model;
  std::getline(model_file, model, '\0');
  if (model.find("Raspberry Pi 4 Model B") != 0) return false;
  return std::filesystem::exists("/usr/local/lib/openhd/nexmon/manifest.json") &&
         std::filesystem::exists("/usr/local/libexec/openhd-nexmon-scout");
}
NexmonScout::NexmonScout() {
  if (std::system((std::string(helper) + "start").c_str()) != 0)
    throw std::runtime_error("Cannot lease internal Wi-Fi for Nexmon scan");
  m_active = true;
}
NexmonScout::~NexmonScout() { restore(); }
bool NexmonScout::restore() {
  if (!m_active) return true;
  if (std::system((std::string(helper) + "restore").c_str()) != 0) return false;
  m_active = false;
  return true;
}
bool NexmonScout::tune(int frequency_mhz) {
  if (frequency_mhz < 2400 || frequency_mhz > 5900) return false;
  return std::system((std::string(helper) + "tune " +
                      std::to_string(frequency_mhz)).c_str()) == 0;
}

bool accumulate_nexmon_packet(const uint8_t* p, unsigned size, int frequency,
                             NexmonObservation& out) {
  if (size < 24 || p[0] != 0 || le16(p+2) != 24 ||
      p[4] != 0x6f || p[5] || p[6] || p[7]) {
    ++out.malformed_packets;
    return false;
  }
  if (le16(p+18) != frequency) return false;  // queued from the old channel
  unsigned length = size - 24;
  if (p[16] & 0x10) {  // FCS included in capture
    if (length < 4) { ++out.malformed_packets; return false; }
    length -= 4;
  }
  if (length < 10 || (p[16] & 0x40)) return false;  // short or bad FCS
  // Count all decoded traffic. These are untrusted frames: exclusion of our
  // video requires authentication, so do not guess ownership from MAC bytes.
  ++out.foreign_packets;
  static constexpr uint8_t rates[] = {2,4,11,22,12,18,24,36,48,72,96,108};
  const auto rate = std::find(std::begin(rates), std::end(rates), p[17]);
  if (rate == std::end(rates)) {
    ++out.unknown_rate_packets;
    return true;
  }
#ifdef OHD_ENABLE_DEVOURER
  out.decoded_airtime_us += devourer::chanmig::frame_airtime_us(
      static_cast<uint16_t>(rate-std::begin(rates)), length+4, 0, false);
#else
  ++out.unknown_rate_packets;
#endif
  return true;
}

NexmonObservation observe_nexmon(int frequency, int duration_ms) {
  char error[PCAP_ERRBUF_SIZE]{};
  std::unique_ptr<pcap_t, decltype(&pcap_close)> capture(
      pcap_open_live(NexmonScout::monitor_interface, 4096, 0, 50, error), pcap_close);
  if (!capture) throw std::runtime_error(error);
  if (pcap_datalink(capture.get()) != DLT_IEEE802_11_RADIO)
    throw std::runtime_error("Nexmon capture has no radiotap header");
  if (pcap_setnonblock(capture.get(), 1, error) != 0)
    throw std::runtime_error(error);
  const int fd = pcap_get_selectable_fd(capture.get());
  if (fd < 0) throw std::runtime_error("Nexmon capture cannot be polled");
  NexmonObservation result;
  const auto end = std::chrono::steady_clock::now() +
                   std::chrono::milliseconds(duration_ms);
  while (std::chrono::steady_clock::now() < end) {
    pcap_pkthdr* header = nullptr;
    const u_char* data = nullptr;
    const int status = pcap_next_ex(capture.get(), &header, &data);
    if (status < 0) throw std::runtime_error(pcap_geterr(capture.get()));
    if (status == 1) {
      if (header->caplen != header->len) ++result.malformed_packets;
      else accumulate_nexmon_packet(data, header->caplen, frequency, result);
    } else {
      pollfd descriptor{fd, POLLIN, 0};
      poll(&descriptor, 1, 20);
    }
  }
  pcap_stat stats{};
  if (pcap_stats(capture.get(), &stats) != 0 || stats.ps_drop || stats.ps_ifdrop)
    throw std::runtime_error("Nexmon capture dropped packets; sample incomplete");
  return result;
}
}  // namespace openhd
