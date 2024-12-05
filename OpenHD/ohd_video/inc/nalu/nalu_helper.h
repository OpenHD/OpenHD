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

#ifndef OPENHD_NALU_HELPER_H
#define OPENHD_NALU_HELPER_H

#include <unistd.h>

static int find_next_nal(const uint8_t* data, int data_len) {
  int nalu_search_state = 0;
  for (int i = 0; i < data_len; i++) {
    switch (nalu_search_state) {
      case 0:
      case 1:
        if (data[i] == 0)
          nalu_search_state++;
        else
          nalu_search_state = 0;
        break;
      case 2:
      case 3:
        if (data[i] == 0) {
          nalu_search_state++;
        } else if (data[i] == 1) {
          // 0,0,0,1 or 0,0,1
          const int len = nalu_search_state == 2 ? 2 : 3;
          if (i > len) {
            return i - len;
          }
          nalu_search_state = 0;
        } else {
          nalu_search_state = 0;
        }
        break;
      default:
        break;
    }
  }
  return data_len;
}

static std::array<uint8_t, 6> EXAMPLE_AUD = {0, 0, 0, 1, 9, 48};
static std::shared_ptr<std::vector<uint8_t>> get_h264_aud() {
  return std::make_shared<std::vector<uint8_t>>(
      EXAMPLE_AUD.data(), EXAMPLE_AUD.data() + EXAMPLE_AUD.size());
}
static std::array<uint8_t, 6> EXAMPLE_START_CODE = {0, 0, 0, 1};
static std::shared_ptr<std::vector<uint8_t>> get_h264_nalu_start_code() {
  return std::make_shared<std::vector<uint8_t>>(EXAMPLE_START_CODE.begin(),
                                                EXAMPLE_START_CODE.end());
}

#endif  // OPENHD_NALU_HELPER_H
