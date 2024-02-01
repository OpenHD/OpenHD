//
// Created by consti10 on 09.12.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_ENDPOINTS_WBENDPOINT_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_ENDPOINTS_WBENDPOINT_H_

#include "MEndpoint.h"
#include "openhd_link.hpp"

// Abstraction for sending / receiving data on/from the link between air and
// ground unit
class WBEndpoint : public MEndpoint {
 public:
  explicit WBEndpoint(std::shared_ptr<OHDLink> link, std::string TAG);
  ~WBEndpoint();

 private:
  std::shared_ptr<OHDLink> m_link_handle;
  bool sendMessagesImpl(const std::vector<MavlinkMessage>& messages) override;
  std::mutex m_send_messages_mutex;
};

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_ENDPOINTS_WBENDPOINT_H_
