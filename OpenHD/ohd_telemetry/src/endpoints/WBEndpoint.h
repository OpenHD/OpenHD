//
// Created by consti10 on 09.12.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_ENDPOINTS_WBENDPOINT_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_ENDPOINTS_WBENDPOINT_H_

#include "MEndpoint.h"
#include "openhd-telemetry-link-interface.h"

class WBEndpoint : public MEndpoint  {
 public:
  explicit WBEndpoint(std::shared_ptr<openhd::ITransmitReceiveTelemetry> tx_rx_handle,std::string TAG="WBEndpoint");
 private:
  std::shared_ptr<openhd::ITransmitReceiveTelemetry> m_tx_rx_handle;
  bool sendMessagesImpl(const std::vector<MavlinkMessage>& messages) override;
};

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_ENDPOINTS_WBENDPOINT_H_
