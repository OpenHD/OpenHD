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

#ifndef OPENHD_FRAGMENT_HELPER_H
#define OPENHD_FRAGMENT_HELPER_H

#include <unistd.h>

static std::vector<std::shared_ptr<std::vector<uint8_t>>> make_fragments(
    const uint8_t* data, int data_len) {
  std::vector<std::shared_ptr<std::vector<uint8_t>>> fragments;
  int bytes_used = 0;
  const uint8_t* p = data;
  static constexpr auto MAX_FRAGMENT_SIZE = 1024;
  while (true) {
    const int remaining = (int)data_len - bytes_used;
    int len = 0;
    if (remaining > MAX_FRAGMENT_SIZE) {
      len = MAX_FRAGMENT_SIZE;
    } else {
      len = remaining;
    }
    std::shared_ptr<std::vector<uint8_t>> fragment =
        std::make_shared<std::vector<uint8_t>>(p, p + len);
    fragments.emplace_back(fragment);
    p = p + len;
    bytes_used += len;
    if (bytes_used == data_len) {
      break;
    }
  }
  return fragments;
}

static std::vector<std::shared_ptr<std::vector<uint8_t>>> drop_fragments(
    std::vector<std::shared_ptr<std::vector<uint8_t>>> fragments) {
  if (fragments.size() > 4) {
    int random = std::rand();
    if (random % 8 == 0) {
      // fragments.resize(fragments.size()/2);
      fragments.resize(0);
    }
  }
  return fragments;
}

#endif  // OPENHD_FRAGMENT_HELPER_H
