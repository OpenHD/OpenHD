//
// Created by consti10 on 10.12.22.
//

#include "stupid_test_link.h"

StupidTestLink::StupidTestLink(OHDProfile profile, OHDPlatform platform,std::vector<std::shared_ptr<WifiCardHolder>> broadcast_cards,
                               std::shared_ptr<openhd::ActionHandler> opt_action_handler):
m_profile(profile),
m_platform(platform),
m_broadcast_cards(std::move(broadcast_cards)),
m_opt_action_handler(std::move(opt_action_handler))
{

}
