// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "XMavlinkParamProvider.h"
#include "endpoints/TCPEndpoint.h"
#include <atomic>
#include <thread>

namespace openhd::settings {
// Parameter-only service for modes that deliberately have no RF/FC telemetry.
class SettingsMavlinkServer {
 public:
  struct Component { uint8_t id; std::vector<Setting> settings; };
  SettingsMavlinkServer(uint8_t system,std::vector<Component> components,
                       int port=TCPEndpoint::DEFAULT_PORT);
  ~SettingsMavlinkServer();
  SettingsMavlinkServer(const SettingsMavlinkServer&)=delete;
  SettingsMavlinkServer& operator=(const SettingsMavlinkServer&)=delete;
 private:
  std::vector<std::shared_ptr<XMavlinkParamProvider>> m_providers;
  std::unique_ptr<TCPEndpoint> m_endpoint;
  std::mutex m_messages_mutex;
  std::atomic<bool> m_running{true};
  std::thread m_thread;
};
}
