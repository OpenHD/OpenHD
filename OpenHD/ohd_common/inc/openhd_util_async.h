//
// Created by consti10 on 09.01.24.
//

#ifndef OPENHD_OPENHD_UTIL_ASYNC_H
#define OPENHD_OPENHD_UTIL_ASYNC_H

#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace openhd {

/**
 * At some points in openhd we just need to fire up a task asynchronously
 * and don't really care for the result. This class helps with that -
 * though make sure to only do this if there are good reasons !
 */
class AsyncHandle {
 public:
  AsyncHandle();
  ~AsyncHandle();
  static AsyncHandle& instance();
  void execute_async(std::string tag, std::function<void()> runnable);
  void execute_command_async(std::string tag, std::string command);
  int get_n_current_tasks();

 private:
  std::mutex m_threads_mutex;
  struct RunningTask {
    std::shared_ptr<std::thread> worker_thread;
    std::function<void()> runnable;
    std::string tag;
    bool done = false;
    std::chrono::steady_clock::time_point start_time =
        std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point last_watchdog_error_log =
        std::chrono::steady_clock::now();
  };
  std::deque<std::shared_ptr<RunningTask>> m_tasks;
  bool m_watchdog_run = true;
  std::shared_ptr<std::thread> m_watchdog_thread;
  void check_watchdog();
  static bool terminate_when_done(const RunningTask& task);
};
}  // namespace openhd

#endif  // OPENHD_OPENHD_UTIL_ASYNC_H
