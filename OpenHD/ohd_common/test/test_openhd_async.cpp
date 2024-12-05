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
#include "openhd_util_async.h"

int main(int argc, char *argv[]) {
  openhd::AsyncHandle::instance();
  openhd::AsyncHandle::instance().execute_async("LONG_TASK", []() {
    std::this_thread::sleep_for(std::chrono::seconds(10000));
  });
  openhd::AsyncHandle::instance().execute_async("QUICK_TASK", []() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  });
  while (openhd::AsyncHandle::instance().get_n_current_tasks()) {
    // Wait until all tasks are finished
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  return 0;
}