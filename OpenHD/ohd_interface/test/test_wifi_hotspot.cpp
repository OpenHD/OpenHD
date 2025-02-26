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

#include "openhd_spdlog.h"
#include "openhd_util.h"
#include "wifi_hotspot.h"

int main(int argc, char *argv[]) {
  OHDUtil::terminate_if_not_root();

  WiFiCard wifiCard;
  // need to manually paste the stuff in here
  // wifiCard.interface_name="wlx244bfeb71c05";
  // wifiCard.mac="24:4b:fe:b7:1c:05";
  wifiCard.device_name = "wlan0";
  wifiCard.mac = "e4:5f:01:b0:55:92";

  OHDProfile profile{true, "none"};
  WifiHotspot wifiHotspot{profile, wifiCard, openhd::WifiSpace::G2_4};
  OHDUtil::keep_alive_until_sigterm();
  openhd::log::get_default()->debug("test end");
  return 0;
}
