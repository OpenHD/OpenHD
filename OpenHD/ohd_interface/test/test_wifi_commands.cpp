//
// Created by consti10 on 28.03.23.
//

#include "openhd_util.h"
#include "wifi_command_helper.h"
// #include "wifi_command_helper2.h"
#include <utility>
#include <vector>

#include "wb_link_helper.h"
#include "wifi_card_discovery.h"

static void test_all_supported_frequencies(const WiFiCard& card,
                                           const int channel_width) {
  OHDUtil::run_command("dmesg", {"--clear"});
  openhd::log::get_default()->debug("test_all_supported_frequencies begin");
  std::vector<std::pair<int, bool>> results;
  for (auto frequency_mhz : card.get_supported_frequencies_2G_5G()) {
    auto channel = openhd::channel_from_frequency(frequency_mhz).value();
    // if(channel.space==openhd::WifiSpace::G2_4){
    //     continue;
    // }
    if (channel_width == 40) {
      if (!channel.is_legal_any_country_40Mhz) {
        openhd::log::get_default()->debug("Skipping {}, no 40Mhz",
                                          frequency_mhz);
        continue;
      }
    }
    // const auto
    // success=wifi::commandhelper::iw_set_frequency_and_channel_width(card.device_name,frequency_mhz,20);
    const auto success =
        openhd::wb::set_frequency_and_channel_width_for_all_cards(
            frequency_mhz, channel_width, {card});
    results.emplace_back(frequency_mhz, success);
    // Weird, otherwise we might get error resource busy
    std::this_thread::sleep_for(std::chrono::seconds(5));
    // We check dmesg output for driver errors to "validate" success.
    // Until i fixed the HT40+/- stuff, rtl8812bu driver crashed on some
    // channels, and i was using this util to debug.
    const auto dmesg_content_opt = OHDUtil::run_command_out("dmesg");
    openhd::log::get_default()->debug("{}", dmesg_content_opt.value_or("None"));
    OHDUtil::run_command("dmesg", {"--clear"});
    if (dmesg_content_opt.has_value()) {
      const auto& dmesg_content = dmesg_content_opt.value();
      if (OHDUtil::contains(dmesg_content, "Call Trace:")) {
        openhd::log::get_default()->debug("{}", dmesg_content);
        break;
      }
    }
  }
  for (const auto& res : results) {
    const int freq = res.first;
    const bool success = res.second;
    if (success) {
      openhd::log::get_default()->debug("Set {} Success", freq);
    } else {
      openhd::log::get_default()->debug("Set {} Error", freq);
    }
  }
  openhd::log::get_default()->debug("test_all_supported_frequencies end");
}

int main(int argc, char* argv[]) {
  OHDUtil::terminate_if_not_root();

  // Hard coded for testing
  // const std::string card_name="wlx244bfeb71c05";
  // const std::string card_name="wlxac9e17596103";
  const std::string card_name = "wlx200db0c3a53c";

  auto card_opt = DWifiCards::process_card(card_name);
  if (!card_opt.has_value()) {
    openhd::log::get_default()->warn("Cannot find card {}", card_name);
    return 0;
  }
  auto card = card_opt.value();
  write_wificards_manifest({card});

  // Take over the card
  wifi::commandhelper::nmcli_set_device_managed_status(card.device_name, false);
  std::this_thread::sleep_for(std::chrono::seconds(2));
  wifi::commandhelper::iw_enable_monitor_mode(card.device_name);
  std::this_thread::sleep_for(std::chrono::seconds(2));
  // debug - set all channel(s) while a (high) tx power is set
  if (card.type == WiFiCardType::OPENHD_RTL_88X2AU) {
    // TODO
  } else {
    const auto tx_power_mbm = openhd::milli_watt_to_mBm(2000);
    wifi::commandhelper::openhd_driver_set_tx_power(card.device_name,
                                                    tx_power_mbm);
  }
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // test_all_supported_frequencies(card,20);
  // std::this_thread::sleep_for(std::chrono::seconds(2));
  test_all_supported_frequencies(card, 40);
  // wifi::commandhelper2::exp_set_wifi_frequency
  // wifi::commandhelper::iw_set_frequency_and_channel_width(card.device_name,5180,20);
  // std::this_thread::sleep_for(std::chrono::seconds(2));
  // wifi::commandhelper::iw_set_frequency_and_channel_width(card.device_name,5200,20);

  // wifi::commandhelper::iw_set_frequency_and_channel_width(card.device_name,5340,20);
  // wifi::commandhelper2::set_wifi_frequency_and_log_result(card.device_name,5340,20);

  // OHDUtil::keep_alive_until_sigterm();

  // Give it back
  wifi::commandhelper::nmcli_set_device_managed_status(card.device_name, true);
}