//
// Created by consti10 on 10.12.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_STUPID_TEST_LINK_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_STUPID_TEST_LINK_H_

#include "openhd-telemetry-tx-rx.h"

#include "openhd-action-handler.hpp"
#include "openhd-platform.hpp"
#include "openhd-profile.hpp"
#include "openhd-spdlog.hpp"
#include "wifi_card.hpp"

// testing only
class StupidTestLink{
 public:
  StupidTestLink(OHDProfile profile,OHDPlatform platform,std::vector<std::shared_ptr<WifiCardHolder>> broadcast_cards,
                 std::shared_ptr<openhd::ActionHandler> opt_action_handler);
  // This handle is used by ohd_telemetry to get / sent telemetry (raw) data
  std::shared_ptr<openhd::TxRxTelemetry> get_telemetry_tx_rx_interface();
 private:

 private:
  const OHDProfile m_profile;
  const OHDPlatform m_platform;
  std::vector<std::shared_ptr<WifiCardHolder>> m_broadcast_cards;
  std::shared_ptr<openhd::ActionHandler> m_opt_action_handler=nullptr;
  std::shared_ptr<spdlog::logger> m_console;
};

#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_STUPID_TEST_LINK_H_
