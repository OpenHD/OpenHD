//
// Created by consti10 on 09.12.22.
//

#include "WBEndpoint.h"

#include <utility>

#include "openhd_spdlog_include.h"

WBEndpoint::WBEndpoint(std::shared_ptr<OHDLink> link, std::string TAG)
    : MEndpoint(std::move(TAG)), m_link_handle(std::move(link)) {
  // assert(m_tx_rx_handle);
  if (!m_link_handle) {
    openhd::log::get_default()->warn(
        "WBEndpoint-tx rx handle is missing (no telemetry connection between "
        "air and ground)");
  } else {
    auto cb = [this](std::shared_ptr<std::vector<uint8_t>> data) {
      MEndpoint::parseNewData(data->data(), data->size());
    };
    m_link_handle->register_on_receive_telemetry_data_cb(cb);
  }
}

WBEndpoint::~WBEndpoint() {
  if (m_link_handle) {
    m_link_handle->register_on_receive_telemetry_data_cb(nullptr);
  }
}

bool WBEndpoint::sendMessagesImpl(const std::vector<MavlinkMessage>& messages) {
  auto message_buffers = aggregate_pack_messages(messages);
  for (const auto& message_buffer : message_buffers) {
    if (m_link_handle) {
      std::lock_guard<std::mutex> guard(m_send_messages_mutex);
      m_link_handle->transmit_telemetry_data(
          {message_buffer.aggregated_data,
           message_buffer.recommended_n_retransmissions});
    }
  }
  return true;
}
