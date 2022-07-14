//
// Created by consti10 on 19.04.22.
//

#ifndef XMAVLINKSERVICE_INTERNALTELEMETRY_H
#define XMAVLINKSERVICE_INTERNALTELEMETRY_H

#include "../mav_helper.h"
#include "routing/MavlinkComponent.hpp"
#include "routing/MavlinkSystem.hpp"
// wifibroadcast header-only
#include <map>
#include <mutex>
#include <optional>
#include <queue>
#include <vector>

#include "HelperSources/SocketHelper.hpp"
#include "OpenHDStatisticsWriter.hpp"
#include "StatusTextAccumulator.hpp"
#include "openhd-log.hpp"
#include "openhd-platform.hpp"
#include "openhd-link-statistics.hpp"

// This Component runs on both the air and ground unit and should handle as many messages / commands / create as many
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
  explicit OHDMainComponent(OHDPlatform platform,uint8_t parent_sys_id,bool runsOnAir);
  // override from component
  std::vector<MavlinkMessage> generate_mavlink_messages() override;
  // override from component
  std::vector<MavlinkMessage> process_mavlink_message(const MavlinkMessage &msg)override;
  // update stats from ohd_interface
  void set_link_statistics(openhd::link_statistics::AllStats stats);
 private:
  const bool RUNS_ON_AIR;
  const OHDPlatform platform;
  // by the sys id QGroundControl knows if this message is telemetry data from the air pi or ground pi.
  // just for convenience, the RUNS_ON_AIR variable determines the sys id.
  //const uint8_t mSysId;
  //const uint8_t mCompId=0;
  // Here all the wb rx instance(s) send their statistics to.
  std::unique_ptr<SocketHelper::UDPReceiver> wifibroadcastStatisticsUdpReceiver;
  // for each unique stream ID, store the last received statistics message.
  // Probably best to go for a write - read, since we don't want to perform any
  // dangerous work on the main service thread.
  std::map<uint8_t, OpenHDStatisticsWriter::Data> lastWbStatisticsMessage;
  /**
   * Called with the raw Wifibroadcast statistics data from UDP
   */
  void processWifibroadcastStatisticsData(const uint8_t *payload, std::size_t payloadSize);
  [[nodiscard]] std::vector<MavlinkMessage> generateWifibroadcastStatistics();
  [[nodiscard]] MavlinkMessage generateOpenHDVersion()const;
  // pack all the buffered log messages
  std::vector<MavlinkMessage> generateLogMessages();
  // here all the log messages are sent to - not in their mavlink form yet.
  std::unique_ptr<SocketHelper::UDPReceiver> logMessagesReceiver;
  StatusTextAccumulator _status_text_accumulator;
  std::mutex _last_link_stats_mutex;
  openhd::link_statistics::AllStats _last_link_stats{};
};

#endif //XMAVLINKSERVICE_INTERNALTELEMETRY_H
