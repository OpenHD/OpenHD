//
// Created by consti10 on 28.03.23.
//

#include "openhd_util.h"
#include "wifi_command_helper.h"
#include "wifi_command_helper2.h"
#include "wifi_card_discovery.h"

int main(int argc, char *argv[]) {
  OHDUtil::terminate_if_not_root();

  // Hard coded for testing
  const std::string card_name="wlx244bfeb71c05";

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
  //wifi::commandhelper::iw_set_frequency_and_channel_width(card.device_name,5340,20);

  wifi::commandhelper2::set_wifi_frequency_and_log_result(card.device_name,5340,20);

  OHDUtil::keep_alive_until_sigterm();

  // Give it back
  wifi::commandhelper::nmcli_set_device_managed_status(card.device_name, true);

}