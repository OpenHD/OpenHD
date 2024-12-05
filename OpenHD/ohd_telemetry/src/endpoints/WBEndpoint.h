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
