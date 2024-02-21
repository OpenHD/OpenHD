//
// Created by consti10 on 30.01.24.
//

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