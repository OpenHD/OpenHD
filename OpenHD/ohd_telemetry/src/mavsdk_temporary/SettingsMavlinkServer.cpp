// SPDX-License-Identifier: GPL-3.0-only
#include "SettingsMavlinkServer.h"

openhd::settings::SettingsMavlinkServer::SettingsMavlinkServer(
    uint8_t system,std::vector<Component> components,int port) {
  for (const auto& component:components) {
    validate_provided_ids(component.settings);
    auto provider=std::make_shared<XMavlinkParamProvider>(system,component.id,std::chrono::seconds(1));
    auto settings=component.settings;
    openhd::testing::append_dummy_if_empty(settings);
    provider->add_params(settings); provider->set_ready();
    m_providers.push_back(std::move(provider));
  }
  m_endpoint=std::make_unique<TCPEndpoint>(openhd::TCPServer::Config{port});
  m_endpoint->registerCallback([this](std::vector<MavlinkMessage> messages){
    std::lock_guard<std::mutex> lock(m_messages_mutex);
    if(!m_running) return;
    for (const auto& provider:m_providers) m_endpoint->sendMessages(provider->process_mavlink_messages(messages));
  });
  m_thread=std::thread([this]{
    while(m_running) {
      {
        std::lock_guard<std::mutex> lock(m_messages_mutex);
        for (const auto& provider:m_providers) m_endpoint->sendMessages(provider->generate_mavlink_messages());
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  });
}

openhd::settings::SettingsMavlinkServer::~SettingsMavlinkServer() {
  { std::lock_guard<std::mutex> lock(m_messages_mutex); m_running=false; }
  if(m_thread.joinable()) m_thread.join();
  m_endpoint.reset(); // Stop callbacks before destroying providers/backend owners.
}
