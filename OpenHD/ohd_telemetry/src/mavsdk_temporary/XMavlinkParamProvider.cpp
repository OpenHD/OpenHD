//
// Created by consti10 on 07.07.22.
//

#include "XMavlinkParamProvider.h"


XMavlinkParamProvider::XMavlinkParamProvider(uint8_t sys_id, uint8_t comp_id,std::shared_ptr<openhd::XSettingsComponent> handler):
MavlinkComponent(sys_id,comp_id),_handler(std::move(handler)){
  _sender=std::make_shared<mavsdk::SenderWrapper>(*this);
  _mavlink_message_handler=std::make_shared<mavsdk::MavlinkMessageHandler>();
  _mavlink_parameter_receiver=
      std::make_shared<mavsdk::MavlinkParameterReceiver>(*_sender,*_mavlink_message_handler);
  const auto settings=_handler->get_all_settings();
  for(const auto& setting:settings) {
	if (std::holds_alternative<openhd::IntSetting>(setting.setting)) {
	  const auto intSetting=std::get<openhd::IntSetting>(setting.setting);
	  const auto result = _mavlink_parameter_receiver->provide_server_param<int>(setting.id,intSetting.value,intSetting.change_callback);
	  assert(result == mavsdk::MavlinkParameterReceiver::Result::Success);
	} else if (std::holds_alternative<openhd::FloatSetting>(setting.setting)) {
	  const auto floatSetting=std::get<openhd::FloatSetting>(setting.setting);
	  const auto result = _mavlink_parameter_receiver->provide_server_param<float>(setting.id,floatSetting.value,floatSetting.change_callback);
	  assert(result == mavsdk::MavlinkParameterReceiver::Result::Success);
	} else if (std::holds_alternative<openhd::StringSetting>(setting.setting)) {
	  const auto stringSetting=std::get<openhd::StringSetting>(setting.setting);
	  const auto result = _mavlink_parameter_receiver->provide_server_param<std::string>(setting.id,stringSetting.value,stringSetting.change_callback);
	  assert(result == mavsdk::MavlinkParameterReceiver::Result::Success);
	} else {
	  assert(true);
	}
  }
  _mavlink_parameter_receiver->ready_for_communication();
}

std::vector<MavlinkMessage> XMavlinkParamProvider::process_mavlink_message(
    const MavlinkMessage& msg) {
  std::lock_guard<std::mutex> lock(_mutex);
  _mavlink_message_handler->process_message(msg.m);
  for(int i=0;i<100;i++){
    _mavlink_parameter_receiver->do_work();
  }
  auto msges=_sender->messages;
  //std::cout<<"XMavlinkParamProvider::process_mavlink_message:"<<msges.size()<<"\n";
  _sender->messages.clear();
  return msges;
}

std::vector<MavlinkMessage> XMavlinkParamProvider::generate_mavlink_messages() {
  std::vector<MavlinkMessage> ret;
  ret.push_back(MavlinkComponent::create_heartbeat());
  return ret;
}
