//
// Created by consti10 on 19.04.22.
//

#ifndef XMAVLINKSERVICE_INTERNALTELEMETRY_H
#define XMAVLINKSERVICE_INTERNALTELEMETRY_H

#include <atomic>
#include <map>
#include <mutex>
#include <optional>
#include <queue>
#include <vector>

#include "../mav_helper.h"
#include "OnboardComputerStatusProvider.h"
#include "last_known_position/LastKnowPosition.h"
#include "openhd_action_handler.h"
#include "openhd_link_statistics.hpp"
#include "openhd_platform.h"
#include "openhd_spdlog.h"
#include "routing/MavlinkComponent.hpp"
#include "routing/MavlinkSystem.hpp"

// This Component runs on both the air and ground unit and should handle as many
// messages / commands / create as many
// "fire and forget" messages as possible. For example, it broadcast the CPU
// load and other statistics, and responds to ping messages. However, external
// OpenHD libraries might create their own component(s), for example Video
// creates a component for each camera and handles commands itself, but then
// uses OpenHD Telemetry to receive / send messages. Quick list of things this
// component does: (might be more if someone forgets to update the documentation
// here) 1) Send Heartbeats 2) Sends out onboard computer status statistics (CPU
// usage, ...) 3) accumulate and send out log messages 4) accumulate and send
// out wifibroadcast stats 5) send out OpenHD version (version of this OpenHD
// release / build ) Note: Sending in this context means they are returned by
// generate_mavlink_messages() and then send out in the upper level.
class OHDMainComponent : public MavlinkComponent {
 public:
  explicit OHDMainComponent(uint8_t parent_sys_id, bool runsOnAir);
  ~OHDMainComponent();
  // override from component
  std::vector<MavlinkMessage> generate_mavlink_messages() override;
  // override from component
  std::vector<MavlinkMessage> process_mavlink_messages(
      std::vector<MavlinkMessage> messages) override;
  void process_command_self(const mavlink_command_long_t& command,
                            int source_sys_id, int source_comp_id,
                            std::vector<MavlinkMessage>& message_buffer);
  // Some features rely on the arming state of the FC, like adjusting tx power &
  // Some features rely on (RC) channel switches, like changing the mcs index
  void check_fc_messages_for_actions(
      const std::vector<MavlinkMessage>& messages);
  std::optional<MavlinkMessage> handle_timesync_message(
      const MavlinkMessage& message);

 private:
  const bool RUNS_ON_AIR;
  // Interval in between heartbeats
  const std::chrono::milliseconds m_heartbeats_interval;
  std::chrono::steady_clock::time_point m_last_heartbeat =
      std::chrono::steady_clock::now();
  std::optional<MavlinkMessage> create_heartbeat_if_needed();
  // We have different intervals on air and ground between the different
  // messages. Always needs to be lower than the telemetry main loop interval.
  const std::chrono::milliseconds m_onboard_computer_status_interval;
  std::chrono::steady_clock::time_point m_last_onboard_computer =
      std::chrono::steady_clock::now();
  // AIR / GND publishes version in 1 second interval
  const std::chrono::milliseconds m_version_message_interval =
      std::chrono::seconds(1);
  std::chrono::steady_clock::time_point m_last_version_message_tp =
      std::chrono::steady_clock::now();
  const std::chrono::milliseconds m_wb_stats_interval;
  std::chrono::steady_clock::time_point m_last_wb_stats =
      std::chrono::steady_clock::now();
  std::vector<MavlinkMessage> create_broadcast_stats_if_needed();
  [[nodiscard]] std::vector<MavlinkMessage> generate_mav_wb_stats();
  [[nodiscard]] MavlinkMessage generate_ohd_version() const;
  // pack all the buffered log messages
  std::vector<MavlinkMessage> generateLogMessages();
  std::unique_ptr<OnboardComputerStatusProvider>
      m_onboard_computer_status_provider;
  MavlinkMessage ack_command(uint8_t source_sys_id, uint8_t source_comp_id,
                             uint16_t command_id, bool success = true);
  std::shared_ptr<spdlog::logger> m_console;
  std::unique_ptr<LastKnowPosition> m_last_known_position = nullptr;
  // Only set / used on air, where we have a uart connection to the FC and
  // therefore can be 100% sure about the FC sys id
  std::atomic_int16_t m_air_fc_sys_id = -1;

 private:
  std::vector<MavlinkMessage> perform_time_synchronisation();
  std::chrono::steady_clock::time_point m_last_timesync_request =
      std::chrono::steady_clock::now();
  int64_t m_last_timesync_out_us = 0;
  void handle_timesync_response_self(const mavlink_timesync_t& tsync);
  int m_good_timesync_offset_count = 0;
  int64_t m_good_timesync_offset_total = 0;
  std::atomic_bool m_has_synced_time = false;
};

#endif  // XMAVLINKSERVICE_INTERNALTELEMETRY_H
