//
// Created by consti10 on 07.07.22.
//

#include "XMavlinkParamProvider.h"

static mavsdk::ParamValue convert_to_mavsd(const openhd::SettingsVariant& setting){
  mavsdk::ParamValue param_value;
  if(std::holds_alternative<int>(setting)){
    param_value.set(std::get<int>(setting));
  }else if(std::holds_alternative<float>(setting)){
    param_value.set(std::get<float>(setting));
  }else if(std::holds_alternative<std::string>(setting)){
    param_value.set(std::get<std::string>(setting));
  }else{
    assert(true);
  }
  return param_value;
}

XMavlinkParamProvider::XMavlinkParamProvider(uint8_t sys_id, uint8_t comp_id,
                                             std::shared_ptr<openhd::XSettingsComponent> handler):
                                                                                                    MavlinkComponent(sys_id,comp_id),_handler(std::move(handler)){
  _sender=std::make_shared<mavsdk::SenderWrapper>(*this);
  _mavlink_message_handler=std::make_shared<mavsdk::MavlinkMessageHandler>();
  _mavlink_parameter_receiver=
      std::make_shared<mavsdk::MavlinkParameterReceiver>(*_sender,*_mavlink_message_handler);
  const auto settings=_handler->get_all_settings();
  for(const auto setting:settings){
    mavsdk::ParamValue param_value;
    if(std::holds_alternative<int>(setting.value)){
      param_value.set(std::get<int>(setting.value));
      _mavlink_parameter_receiver->subscribe_param_int_changed(setting.id,[this,setting](int value){
            this->_handler->process_setting_changed({setting.id,value});
          },this);
    }else if(std::holds_alternative<float>(setting.value)){
      param_value.set(std::get<float>(setting.value));
      _mavlink_parameter_receiver->subscribe_param_float_changed(setting.id,[this,setting](float value){
            this->_handler->process_setting_changed({setting.id,value});
          },this);
    }else if(std::holds_alternative<std::string>(setting.value)){
      param_value.set(std::get<std::string>(setting.value));
      _mavlink_parameter_receiver->subscribe_param_custom_changed(setting.id,[this,setting](std::string value){
            this->_handler->process_setting_changed({setting.id,value});
          },this);
    }else{
      assert(true);
    }
    _mavlink_parameter_receiver->provide_server_param(setting.id,param_value);
  }
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
