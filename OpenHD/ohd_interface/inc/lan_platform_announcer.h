#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

class LanPlatformAnnouncer {
 public:
  explicit LanPlatformAnnouncer(bool is_air);
  ~LanPlatformAnnouncer();

 private:
  void run(bool is_air);

  std::mutex m_mutex;
  std::condition_variable m_stop_requested;
  bool m_stopping = false;
  std::thread m_thread;
};
