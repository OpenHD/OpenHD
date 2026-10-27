/******************************************************************************
 * OpenHD
 *
 * Licensed under the GNU General Public License (GPL) Version 3.
 *
 * This software is provided "as-is," without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose, and non-infringement. For details, see the
 * full license in the LICENSE file provided with this source code.
 *
 * Non-Military Use Only:
 * This software and its associated components are explicitly intended for
 * civilian and non-military purposes. Use in any military or defense
 * applications is strictly prohibited unless explicitly and individually
 * licensed otherwise by the OpenHD Team.
 *
 * Contributors:
 * A full list of contributors can be found at the OpenHD GitHub repository:
 * https://github.com/OpenHD
 *
 * © OpenHD, All Rights Reserved.
 ******************************************************************************/

#include "MEndpoint.h"
#include "../legacy_radio_settings.h"

#include "openhd_spdlog_include.h"

// WARNING BE CAREFULL TO REMOVE ON RELEASE
// #define OHD_TELEMETRY_TESTING_ENABLE_PACKET_LOSS

MEndpoint::MEndpoint(std::string tag, bool debug_mavlink_msg_packet_loss)
    : TAG(std::move(tag)),
      m_debug_mavlink_msg_packet_loss(debug_mavlink_msg_packet_loss) {
  openhd::log::get_default()->debug(
      "{} using instance parser debug_mavlink_msg_packet_loss:{}", TAG,
      m_debug_mavlink_msg_packet_loss);
}

void MEndpoint::sendMessages(const std::vector<MavlinkMessage>& messages) {
  if (messages.empty()) return;
  m_tx_n_bytes += get_size(messages);
  /*for(const auto& msg: messages){
    if(msg.m.msgid==MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE){
      openhd::log::get_default()->debug("Send rc channels override");
    }
  }*/
  // openhd::log::create_or_get(TAG)->debug("N messages
  // send:{}",messages.size());
  const auto res = sendMessagesImpl(messages);
  for (const auto& message : messages)
    m_diagnostics.record(message.m, res ? TelemetryDiagnostics::TxAccepted
                                      : TelemetryDiagnostics::TxRejected);
  m_diagnostics.log_if_due(TAG);
  m_n_messages_sent += messages.size();
  if (!res) {
    m_n_messages_send_failed += messages.size();
  }
}

void MEndpoint::registerCallback(MAV_MSG_CALLBACK cb) {
  if (m_callback != nullptr) {
    // this might be a common programming mistake - you can only register one
    // callback here
    openhd::log::get_default()->warn("Overwriting already existing callback");
  }
  m_callback = std::move(cb);
}

bool MEndpoint::isAlive() const {
  return (std::chrono::steady_clock::now() - lastMessage) <
         std::chrono::seconds(5);
}

std::string MEndpoint::createInfo() const {
  std::stringstream ss;
  ss << TAG << " {sent:" << m_n_messages_sent
     << " send_failed:" << m_n_messages_send_failed
     << " recv:" << m_n_messages_received
     << " alive:" << (isAlive() ? "Y" : "N") << "}\n";
  return ss.str();
}

void MEndpoint::parseNewData(const uint8_t* data, const int data_len) {
  //<<TAG<<" received data:"<<data_len<<"
  //"<<MavlinkHelpers::raw_content(data,data_len)<<"\n";
  m_rx_n_bytes += data_len;
  std::vector<MavlinkMessage> messages;
  mavlink_message_t msg;
  for (int i = 0; i < data_len; i++) {
    uint8_t framing = MAVLINK_FRAMING_INCOMPLETE;
    uint8_t res = openhd_parse_buffer(&m_parser_buffer, &m_parser_status, (uint8_t)data[i], &msg,
                                     &receiveMavlinkStatus, &framing);
    if (framing == MAVLINK_FRAMING_BAD_CRC)
      m_diagnostics.record(msg, TelemetryDiagnostics::BadCrc);
    else if (framing == MAVLINK_FRAMING_BAD_SIGNATURE)
      m_diagnostics.record(msg, TelemetryDiagnostics::BadSignature);
    if (res) {
      messages.push_back(MavlinkMessage{msg});
      // From
      // https://github.com/mavlink/c_uart_interface_example/blob/master/serial_port.cpp
      if ((m_last_status.packet_rx_drop_count !=
           receiveMavlinkStatus.packet_rx_drop_count) &&
          m_debug_mavlink_msg_packet_loss) {
        openhd::log::get_default()->warn(
            "DROPPED {} PACKETS", receiveMavlinkStatus.packet_rx_drop_count);
      }
      m_last_status = receiveMavlinkStatus;
    }
  }
  onNewMavlinkMessages(messages);
  m_diagnostics.log_if_due(TAG);
}

void MEndpoint::onNewMavlinkMessages(std::vector<MavlinkMessage> messages) {
  if (messages.empty()) return;
  for (const auto& message : messages)
    m_diagnostics.record(message.m, TelemetryDiagnostics::Rx);
  // openhd::log::create_or_get(TAG)->debug("N messages
  // receive:{}",messages.size());
  lastMessage = std::chrono::steady_clock::now();
  m_n_messages_received += messages.size();
  if (m_callback != nullptr) {
    const auto begin = std::chrono::steady_clock::now();
    m_callback(messages);
    m_diagnostics.callback_time(std::chrono::steady_clock::now() - begin);
  } else {
    openhd::log::get_default()->warn(
        "No callback set,did you forget to add it ?");
  }
}
