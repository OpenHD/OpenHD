//
// Created by consti10 on 07.07.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAV_PARAM_XMAVLINKPARAMPROVIDER_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAV_PARAM_XMAVLINKPARAMPROVIDER_H_

#include <memory>
#include <optional>
#include <utility>

#include "openhd_settings_imp.h"
#include "routing/MavlinkComponent.hpp"

// mavsdk
#include "mavlink_message_handler.h"
#include "mavlink_parameter_receiver.h"
#include "sender_wrapper.h"

class XMavlinkParamProvider : public MavlinkComponent {
 public:
  // !!!! Note : no params are active until set_ready() is called
  // This way, there is no parameter invariance, but it is easy to forget to
  // call set_read() !!!
  explicit XMavlinkParamProvider(uint8_t sys_id, uint8_t comp_id,
                                 std::optional<std::chrono::milliseconds>
                                     opt_heartbeat_interval = std::nullopt);
  void add_param(const openhd::Setting& setting);
  // only usable when manually_set_ready is true
  void add_params(const std::vector<openhd::Setting>& settings);
  void set_ready();
  // override from component
  std::vector<MavlinkMessage> process_mavlink_messages(
      std::vector<MavlinkMessage> messages) override;
  // override from component
  std::vector<MavlinkMessage> generate_mavlink_messages() override;

 private:
  // mavsdk
  std::shared_ptr<mavsdk::SenderWrapper> _sender;
  std::shared_ptr<mavsdk::MavlinkMessageHandler> _mavlink_message_handler;
  std::shared_ptr<mavsdk::MavlinkParameterReceiver> _mavlink_parameter_receiver;

 private:
  std::mutex _mutex{};
  const std::optional<std::chrono::milliseconds> m_opt_heartbeat_interval;
  std::chrono::steady_clock::time_point m_last_heartbeat =
      std::chrono::steady_clock::now();
  // Dirty, when openhd updates a setting
  std::vector<openhd::Setting> m_int_settings_with_update_functionality;
};

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAV_PARAM_XMAVLINKPARAMPROVIDER_H_
