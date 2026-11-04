#pragma once
#include <chrono>
namespace openhd::wb {
// Old-channel announcements can arrive between PREPARE and the air retune.
// Keep them from undoing the ground switch, but allow failed air retunes to
// recover after the preparation window.
class ChannelSwitchAnnouncementGuard {
 public:
  using Clock = std::chrono::steady_clock;
  void prepare(int frequency, int width, Clock::time_point now) {
    frequency_ = frequency;
    width_ = width;
    until_ = now + std::chrono::seconds(2);
    active_ = true;
  }
  bool accept(int frequency, int width, Clock::time_point now) {
    if (!active_) return true;
    if (frequency == frequency_ && width == width_) return true;
    // A delayed old-channel packet can follow the first target packet.
    // Keep guarding until the window expires, or a new PREPARE supersedes it.
    if (now < until_) return false;
    active_ = false;
    return true;
  }
 private:
  int frequency_ = 0, width_ = 0;
  bool active_ = false;
  Clock::time_point until_{};
};
}  // namespace openhd::wb
