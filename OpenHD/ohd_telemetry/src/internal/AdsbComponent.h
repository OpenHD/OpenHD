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

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_ADSBCOMPONENT_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_ADSBCOMPONENT_H_

#include <atomic>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "../mav_helper.h"
#include "openhd_spdlog.h"
#include "routing/MavlinkComponent.hpp"
#include "routing/MavlinkSystem.hpp"

class AdsbComponent : public MavlinkComponent {
 public:
  explicit AdsbComponent(uint8_t parent_sys_id);
  ~AdsbComponent();

  std::vector<MavlinkMessage> generate_mavlink_messages() override;
  std::vector<MavlinkMessage> process_mavlink_messages(
      std::vector<MavlinkMessage> messages) override;

 private:
  void process_runner();
  void tcp_client_runner();

  std::shared_ptr<spdlog::logger> m_console;
  std::atomic_bool m_terminate{false};
  std::thread m_process_thread;
  std::thread m_tcp_thread;

  // ICAO -> ADSB Vehicle Message
  std::mutex m_adsb_mutex;
  std::map<uint32_t, mavlink_adsb_vehicle_t> m_adsb_vehicles;
};

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_ADSBCOMPONENT_H_
