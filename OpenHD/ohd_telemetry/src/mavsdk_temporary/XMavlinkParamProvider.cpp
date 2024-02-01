//
// Created by consti10 on 07.07.22.
//

#include "XMavlinkParamProvider.h"

#include <openhd_spdlog.h>

XMavlinkParamProvider::XMavlinkParamProvider(
    uint8_t sys_id, uint8_t comp_id,
    std::optional<std::chrono::milliseconds> opt_heartbeat_interval)
    : MavlinkComponent(sys_id, comp_id),
      m_opt_heartbeat_interval(opt_heartbeat_interval) {
  _sender = std::make_shared<mavsdk::SenderWrapper>(*this);
  _mavlink_message_handler = std::make_shared<mavsdk::MavlinkMessageHandler>();
  _mavlink_parameter_receiver =
      std::make_shared<mavsdk::MavlinkParameterReceiver>(
          *_sender, *_mavlink_message_handler);
}

void XMavlinkParamProvider::add_param(const openhd::Setting& setting) {
  if (std::holds_alternative<openhd::IntSetting>(setting.setting)) {
    const auto intSetting = std::get<openhd::IntSetting>(setting.setting);
    const auto result = _mavlink_parameter_receiver->provide_server_param<int>(
        setting.id, intSetting.value, intSetting.change_callback);
    assert(result == mavsdk::MavlinkParameterReceiver::Result::Success);
    if (intSetting.get_callback != nullptr) {
      m_int_settings_with_update_functionality.push_back(setting);
    }
  } else if (std::holds_alternative<openhd::StringSetting>(setting.setting)) {
    const auto stringSetting = std::get<openhd::StringSetting>(setting.setting);
    const auto result =
        _mavlink_parameter_receiver->provide_server_param<std::string>(
            setting.id, stringSetting.value, stringSetting.change_callback);
    assert(result == mavsdk::MavlinkParameterReceiver::Result::Success);
  } else {
    assert(false);
  }
}

void XMavlinkParamProvider::add_params(
    const std::vector<openhd::Setting>& settings) {
  for (const auto& setting : settings) {
    add_param(setting);
  }
}

void XMavlinkParamProvider::set_ready() {
  _mavlink_parameter_receiver->ready_for_communication();
}

std::vector<MavlinkMessage> XMavlinkParamProvider::process_mavlink_messages(
    std::vector<MavlinkMessage> messages) {
  std::lock_guard<std::mutex> lock(_mutex);
  for (const auto& setting : m_int_settings_with_update_functionality) {
    const auto intSetting = std::get<openhd::IntSetting>(setting.setting);
    const auto currValue =
        _mavlink_parameter_receiver->retrieve_server_param_int(setting.id);
    if (currValue.first == mavsdk::MavlinkParameterReceiver::Result::Success) {
      const int currValueInt = currValue.second;
      const int newIntvalue = intSetting.get_callback();
      if (currValueInt != newIntvalue) {
        // Param set on ground is now different to the one inside the gcs
        openhd::log::get_default()->warn("Updating {} from {} to {}",
                                         setting.id, currValueInt, newIntvalue);
        _mavlink_parameter_receiver->update_existing_server_param_int(
            setting.id, newIntvalue);
      }
    }
  }
  for (const auto& msg : messages) {
    _mavlink_message_handler->process_message(msg.m);
  }
  for (int i = 0; i < 100; i++) {
    _mavlink_parameter_receiver->do_work();
  }
  auto msges = _sender->messages;
  // std::cout<<"XMavlinkParamProvider::process_mavlink_message:"<<msges.size()<<"\n";
  _sender->messages.clear();
  return msges;
}

std::vector<MavlinkMessage> XMavlinkParamProvider::generate_mavlink_messages() {
  std::vector<MavlinkMessage> ret;
  if (m_opt_heartbeat_interval.has_value()) {
    const auto now = std::chrono::steady_clock::now();
    const auto elapsed = now - m_last_heartbeat;
    if (elapsed > m_opt_heartbeat_interval.value()) {
      m_last_heartbeat = now;
      ret.push_back(MavlinkComponent::create_heartbeat());
    }
  }
  return ret;
}
