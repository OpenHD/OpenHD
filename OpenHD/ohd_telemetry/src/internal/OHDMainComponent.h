//
// Created by consti10 on 19.04.22.
//

#ifndef XMAVLINKSERVICE_INTERNALTELEMETRY_H
#define XMAVLINKSERVICE_INTERNALTELEMETRY_H

#include <map>
#include <mutex>
#include <optional>
#include <queue>
#include <vector>

#include "../mav_helper.h"
#include "HelperSources/SocketHelper.hpp"
#include "OnboardComputerStatusProvider.h"
#include "WBReceiverStats.hpp"
#include "StatusTextAccumulator.hpp"
#include "openhd-action-handler.hpp"
#include "openhd-link-statistics.hpp"
#include "openhd-platform.hpp"
#include "openhd-spdlog.hpp"
#include "openhd-spdlog-tele-sink.h"
#include "routing/MavlinkComponent.hpp"
#include "routing/MavlinkSystem.hpp"

// This Component runs on both the air and ground unit and should handle as many
// messages / commands / create as many
// "fire and forget" messages as possible. For example, it broadcast the CPU load and other statistics, and responds to ping messages.
// However, external OpenHD libraries might create their own component(s), for example
// Video creates a component for each camera and handles commands itself, but then uses OpenHD Telemetry to receive / send messages.
// Quick list of things this component does: (might be more if someone forgets to update the documentation here)
// 1) Send Heartbeats
// 2) Sends out onboard computer status statistics (CPU usage, ...)
// 3) accumulate and send out log messages
// 4) accumulate and send out wifibroadcast stats
// 5) send out OpenHD version (version of this OpenHD release / build )
// Note: Sending in this context means they are returned by generate_mavlink_messages() and then send out in the upper level.
class OHDMainComponent : public MavlinkComponent{
 public:
  explicit OHDMainComponent(OHDPlatform platform,uint8_t parent_sys_id,bool runsOnAir,std::shared_ptr<openhd::ActionHandler> opt_action_handler);
  // override from component
  std::vector<MavlinkMessage> generate_mavlink_messages() override;
  // override from component
  std::vector<MavlinkMessage> process_mavlink_messages(std::vector<MavlinkMessage> messages)override;
  // update stats from ohd_interface
  void set_link_statistics(openhd::link_statistics::StatsAirGround stats);
  openhd::link_statistics::StatsAirGround get_latest_link_statistics();
 private:
  const bool RUNS_ON_AIR;
  const OHDPlatform m_platform;
  std::shared_ptr<openhd::ActionHandler> m_opt_action_handler;
  // by the sys id QGroundControl knows if this message is telemetry data from the air pi or ground pi.
  // just for convenience, the RUNS_ON_AIR variable determines the sys id.
  //const uint8_t mSysId;
  //const uint8_t mCompId=0;
  // similar to ping
  [[nodiscard]] std::optional<MavlinkMessage> handle_timesync_message(const MavlinkMessage &message);
  [[nodiscard]] std::vector<MavlinkMessage> generate_mav_wb_stats();
  [[nodiscard]] MavlinkMessage generate_ohd_version(const std::string& commit_hash= "unknown")const;
  // pack all the buffered log messages
  std::vector<MavlinkMessage> generateLogMessages();
  // here all the log messages are sent to - not in their mavlink form yet.
  std::unique_ptr<SocketHelper::UDPReceiver> m_log_messages_receiver;
  StatusTextAccumulator m_status_text_accumulator;
  std::unique_ptr<OnboardComputerStatusProvider> m_onboard_computer_status_provider;
  std::mutex m_last_link_stats_mutex;
  openhd::link_statistics::StatsAirGround m_last_link_stats{};
  MavlinkMessage ack_command(uint8_t source_sys_id,uint8_t source_comp_id,uint16_t command_id);
  std::shared_ptr<spdlog::logger> m_console;
};

#endif //XMAVLINKSERVICE_INTERNALTELEMETRY_H
