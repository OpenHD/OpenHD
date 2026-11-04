#include "channel_switch_announcement_guard.h"
#include "wb_link_rate_budget.h"
#include <cassert>
#include "wb_radio_controls.h"
int main() {
  using openhd::effective_tx_power_level;
  static_assert(effective_tx_power_level(true, 20, false, true, 10) == 80);
  static_assert(effective_tx_power_level(true, 100, false, true, 10) == 100);
  static_assert(effective_tx_power_level(true, 150, false, true, 10) == 150);
  static_assert(effective_tx_power_level(true, 0, false, true, 10) == 0);
  static_assert(effective_tx_power_level(true, 100, true, true, 10) == 20);
  static_assert(effective_tx_power_level(false, 60, true, true, 10) == 60);
  static_assert(effective_tx_power_level(false, 20, false, true, 10) == 20);
  static_assert(effective_tx_power_level(true, 20, false, true, 20) == 20);
  static_assert(effective_tx_power_level(true, 20, false, true, 40) == 20);
  static_assert(effective_tx_power_level(true, 20, false, false, 10) == 20);
  using Guard = openhd::wb::ChannelSwitchAnnouncementGuard;
  const auto now = Guard::Clock::now();
  Guard guard;
  assert(guard.accept(5745, 20, now));
  guard.prepare(5765, 20, now);
  assert(!guard.accept(5745, 20, now + std::chrono::milliseconds(100)));
  assert(guard.accept(5765, 20, now + std::chrono::milliseconds(200)));
  assert(!guard.accept(5745, 20, now + std::chrono::milliseconds(250)));
  // A width-only change must reject the old width at the same frequency.
  guard.prepare(5765, 10, now);
  assert(!guard.accept(5765, 20, now + std::chrono::milliseconds(100)));
  assert(guard.accept(5765, 10, now + std::chrono::milliseconds(200)));
  // Failed retunes must not mask old-channel recovery forever.
  guard.prepare(5785, 40, now);
  assert(guard.accept(5765, 20, now + std::chrono::seconds(2)));
  for (int mcs = 0; mcs <= 12; ++mcs) {
    const auto wide = openhd::wb::rtl8812au_get_max_rate_5G_kbits(mcs, 20);
    assert(openhd::wb::rtl8812au_get_max_rate_5G_kbits(mcs, 10) == wide / 2);
  }
}
