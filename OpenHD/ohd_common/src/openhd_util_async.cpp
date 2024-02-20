//
// Created by consti10 on 09.01.24.
//

#include "openhd_util_async.h"

#include <utility>

#include "openhd_spdlog.h"
#include "openhd_util.h"

openhd::AsyncHandle::AsyncHandle() {
  m_watchdog_run = true;
  m_watchdog_thread =
      std::make_unique<std::thread>(&AsyncHandle::check_watchdog, this);
}

openhd::AsyncHandle::~AsyncHandle() {
  m_watchdog_run = false;
  m_watchdog_thread->join();
  for (auto& task : m_tasks) {
    if (!task->done) {
      openhd::log::get_default()->warn("{} probably dead", task->tag);
    }
    if (task->worker_thread->joinable()) {
      task->worker_thread->join();
    }
  }
}

openhd::AsyncHandle& openhd::AsyncHandle::instance() {
  static AsyncHandle instance{};
  return instance;
}

void openhd::AsyncHandle::execute_async(const std::string tag,
                                        std::function<void()> runnable) {
  auto task = std::make_shared<openhd::AsyncHandle::RunningTask>();
  task->tag = tag;
  task->runnable = std::move(runnable);
  task->done = false;
  task->worker_thread = std::make_shared<std::thread>([task]() {
    auto console = openhd::log::get_default();
    console->debug("{} begin", task->tag);
    try {
      task->runnable();
    } catch (std::exception& ex) {
      console->warn("Exception on {},{}", task->tag, ex.what());
    } catch (...) {
      console->warn("Unknown Exception on {}", task->tag);
    }
    console->debug("{} done", task->tag);
    task->done = true;
  });
  std::lock_guard<std::mutex> lock(m_threads_mutex);
  m_tasks.push_back(task);
}

void openhd::AsyncHandle::execute_command_async(std::string tag,
                                                std::string command) {
  auto runnable = [command]() { OHDUtil::run_command(command, {}, true); };
  execute_async(std::move(tag), runnable);
}

bool openhd::AsyncHandle::terminate_when_done(
    const openhd::AsyncHandle::RunningTask& task) {
  if (!task.done) return false;
  if (task.worker_thread->joinable()) {
    task.worker_thread->join();
  }
  return true;
}

void openhd::AsyncHandle::check_watchdog() {
  while (m_watchdog_run) {
    {  // Let the mutex go out of scope before sleeping
      std::lock_guard<std::mutex> lock(m_threads_mutex);
      m_tasks.erase(std::remove_if(m_tasks.begin(), m_tasks.end(),
                                   [](std::shared_ptr<RunningTask>& task) {
                                     // returns true if task is done
                                     return terminate_when_done(*task);
                                   }),
                    m_tasks.end());
      for (auto& task : m_tasks) {
        const auto elapsed_task =
            std::chrono::steady_clock::now() - task->start_time;
        if (elapsed_task > std::chrono::seconds(10)) {
          // Log a warning message every 3 seconds on a (presumably) hanging
          // task
          if (std::chrono::steady_clock::now() - task->last_watchdog_error_log >
              std::chrono::seconds(3)) {
            openhd::log::get_default()->warn("Async Task [{}] hanging ?",
                                             task->tag);
          }
        }
      }
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

int openhd::AsyncHandle::get_n_current_tasks() {
  std::lock_guard<std::mutex> lock(m_threads_mutex);
  return m_tasks.size();
}
