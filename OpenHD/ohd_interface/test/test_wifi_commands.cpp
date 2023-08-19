//
// Created by consti10 on 28.03.23.
//

#include "openhd_util.h"
#include "wifi_command_helper.h"
//#include "wifi_command_helper2.h"
#include "wifi_card_discovery.h"
#include "wb_link_helper.h"

#include <vector>
#include <utility>

static void test_all_supported_frequencies(const WiFiCard& card){
  openhd::log::get_default()->debug("test_all_supported_frequencies begin");
  std::vector<std::pair<int,bool>> results;
  for(auto frequency_mhz: card.supported_frequencies_5G){
    //const auto success=wifi::commandhelper::iw_set_frequency_and_channel_width(card.device_name,frequency_mhz,20);
    const auto success=openhd::wb::set_frequency_and_channel_width_for_all_cards(frequency_mhz,20,{card});
    results.emplace_back(frequency_mhz,success);
    // Weird, otherwise we might get error resource busy
    std::this_thread::sleep_for(std::chrono::seconds (5));
  }
  for(const auto& res:results){
    const int freq=res.first;
    const bool success=res.second;
    if(success){
      openhd::log::get_default()->debug("Set {} Success",freq);
    }else{
      openhd::log::get_default()->debug("Set {} Error",freq);
    }
  }
  openhd::log::get_default()->debug("test_all_supported_frequencies end");
}

int main(int argc, char *argv[]) {
  OHDUtil::terminate_if_not_root();

  // Hard coded for testing
  //const std::string card_name="wlx244bfeb71c05";
  const std::string card_name="wlxac9e17596103";

  auto card_opt = DWifiCards::process_card(card_name);
  if(!card_opt.has_value()){
    openhd::log::get_default()->warn("Cannot find card {}",card_name);
    return 0;
  }
  auto card=card_opt.value();
  write_wificards_manifest({card});

  // Take over the card
  wifi::commandhelper::nmcli_set_device_managed_status(card.device_name, false);

  wifi::commandhelper::iw_enable_monitor_mode(card.device_name);

  test_all_supported_frequencies(card);
  //wifi::commandhelper2::exp_set_wifi_frequency
  //wifi::commandhelper::iw_set_frequency_and_channel_width(card.device_name,5180,20);
  //std::this_thread::sleep_for(std::chrono::seconds(2));
  //wifi::commandhelper::iw_set_frequency_and_channel_width(card.device_name,5200,20);

  //wifi::commandhelper::iw_set_frequency_and_channel_width(card.device_name,5340,20);
  //wifi::commandhelper2::set_wifi_frequency_and_log_result(card.device_name,5340,20);

  //OHDUtil::keep_alive_until_sigterm();

  // Give it back
  wifi::commandhelper::nmcli_set_device_managed_status(card.device_name, true);

}