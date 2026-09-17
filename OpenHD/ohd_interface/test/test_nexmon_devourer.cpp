// Bounded hardware interop test. TX sends real OpenHD management announcements
// through Devourer; RX uses the authenticated decoder on an already leased scout.
// Usage: sudo test_nexmon_devourer tx devourer-usb-1-3 5745 90
//        sudo test_nexmon_devourer rx ohdscout 5745 10
#include "../lib/wifibroadcast/wifibroadcast/src/WBTxRx.h"
#include "openhd_global_constants.hpp"
#include <atomic>
#include <chrono>
#include <cstring>
#include <cstdio>
#include <thread>

static wb::KeyPairTxRx default_keys() {
  static constexpr unsigned char air[] = {
      41,129,7,201,56,11,77,90,223,5,19,41,88,93,100,11};
  static constexpr unsigned char ground[] = {
      92,18,66,220,171,9,14,207,12,44,97,2,51,99,17,221};
  wb::KeyPairTxRx keys{};
  unsigned char seed[crypto_box_SEEDBYTES];
  auto derive = [&](const unsigned char* salt, wb::Key& pair) {
    const char* phrase = "O-p-e-n-H-D";
    if (crypto_pwhash(seed, sizeof(seed), phrase, std::strlen(phrase), salt,
                     crypto_pwhash_OPSLIMIT_INTERACTIVE,
                     crypto_pwhash_MEMLIMIT_INTERACTIVE,
                     crypto_pwhash_ALG_DEFAULT) != 0)
      throw std::runtime_error("Cannot derive default keys");
    crypto_box_seed_keypair(pair.public_key.data(), pair.secret_key.data(), seed);
  };
  derive(air, keys.key_1);
  derive(ground, keys.key_2);
  sodium_memzero(seed, sizeof(seed));
  return keys;
}

int main(int argc, char** argv) {
  try {
    if (argc != 5 || (std::string(argv[1]) != "tx" && std::string(argv[1]) != "rx"))
      throw std::runtime_error("Usage: test_nexmon_devourer tx|rx card frequency seconds");
    const bool tx = std::string(argv[1]) == "tx";
    const int frequency = std::stoi(argv[3]), seconds = std::stoi(argv[4]);
    const bool valid_frequency =
        (frequency >= 2412 && frequency <= 2472 && (frequency-2407) % 5 == 0) ||
        (frequency >= 5000 && frequency <= 5925 && frequency % 5 == 0);
    if (!valid_frequency ||
        seconds < 1 || seconds > 180)
      throw std::runtime_error("Invalid frequency or duration");
    if (sodium_init() < 0) throw std::runtime_error("Cannot initialize sodium");
    WBTxRx::Options options;
    options.secure_keypair = default_keys();
    options.use_gnd_identifier = !tx;
    options.use_devourer = tx;
    options.devourer_frequency_mhz = frequency;
    options.devourer_channel_width_mhz = 20;
    options.enable_auto_switch_tx_card = false;
    options.receive_thread_max_realtime = false;
    auto header = std::make_shared<RadiotapHeaderTxHolder>();
    WBTxRx radio({{argv[2], 0}}, options, header);
    std::atomic<int> announcements{0};
    if (!tx) {
      radio.set_passive_mode(true);
      radio.rx_register_stream_handler(std::make_shared<WBTxRx::StreamRxHandler>(
          openhd::MANAGEMENT_RADIO_PORT_AIR_TX,
          [&](uint64_t, int, const uint8_t* data, int length) {
            uint32_t reported = 0;
            if (length != 6 || data[0] != 0 || data[5] != 20) return;
            std::memcpy(&reported, data + 1, 4);
            if (reported == frequency) ++announcements;
          }, nullptr));
      radio.start_receiving();
    }
    uint8_t packet[6]{0,0,0,0,0,20};
    const uint32_t reported = frequency;
    std::memcpy(packet + 1, &reported, 4);
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(seconds);
    int sent = 0;
    while (std::chrono::steady_clock::now() < deadline) {
      if (tx) {
        radio.tx_inject_packet(openhd::MANAGEMENT_RADIO_PORT_AIR_TX,
                               packet, sizeof(packet), header->thread_safe_get(), true);
        ++sent;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    radio.stop_receiving();
    const auto stats = radio.get_rx_stats();
    std::printf("RESULT frequency=%d sent=%d packets=%lld likely_openhd=%lld authenticated=%lld matching_announcements=%d\n",
                frequency, sent, static_cast<long long>(stats.count_p_any),
                static_cast<long long>(stats.curr_n_likely_openhd_packets),
                static_cast<long long>(stats.count_p_valid), announcements.load());
    std::fflush(stdout);
    return tx || announcements > 0 ? 0 : 2;
  } catch (const std::exception& error) {
    std::fprintf(stderr, "%s\n", error.what());
    return 1;
  }
}
