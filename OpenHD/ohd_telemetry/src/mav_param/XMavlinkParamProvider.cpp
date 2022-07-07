//
// Created by consti10 on 07.07.22.
//

#include "XMavlinkParamProvider.h"


XMavlinkParamProvider::XMavlinkParamProvider(MavlinkSystem& parent, uint8_t comp_id,
    std::shared_ptr<openhd::XSettingsComponent> handler):
      MavlinkComponent(parent,comp_id),_handler(std::move(handler)){
    _sender=std::make_shared<mavsdk::SenderWrapper>();
    _mavlink_message_handler=std::make_shared<mavsdk::MavlinkMessageHandler>();
    _mavlink_parameter_receiver=std::make_shared<mavsdk::MavlinkParameterReceiver>(*_sender,*_mavlink_message_handler);
}

std::vector<MavlinkMessage> XMavlinkParamProvider::process_mavlink_message(
    const MavlinkMessage& msg) {
  _mavlink_message_handler->process_message(msg.m);
  _mavlink_parameter_receiver->do_work();
  auto msges=_sender->messages;
  std::cout<<"XMavlinkParamProvider::process_mavlink_message:"<<msges.size()<<"\n";
  _sender->messages.clear();
  return msges;
}

std::vector<MavlinkMessage> XMavlinkParamProvider::generate_mavlink_messages() {
  return std::vector<MavlinkMessage>();
}
