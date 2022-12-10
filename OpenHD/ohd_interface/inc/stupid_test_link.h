//
// Created by consti10 on 10.12.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_STUPID_TEST_LINK_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_STUPID_TEST_LINK_H_

#include "openhd-platform.hpp"
#include "openhd-profile.hpp"
#include "openhd-spdlog.hpp"
#include "wifi_card.hpp"
#include "openhd-action-handler.hpp"

// testing only
class StupidTestLink{
 public:
  StupidTestLink(OHDProfile profile,OHDPlatform platform,std::vector<std::shared_ptr<WifiCardHolder>> broadcast_cards,
                 std::shared_ptr<openhd::ActionHandler> opt_action_handler);
 private:
 private:
  const OHDProfile m_profile;
  const OHDPlatform m_platform;
  std::vector<std::shared_ptr<WifiCardHolder>> m_broadcast_cards;
  std::shared_ptr<openhd::ActionHandler> m_opt_action_handler=nullptr;
  std::shared_ptr<spdlog::logger> m_console;
};

#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_STUPID_TEST_LINK_H_
