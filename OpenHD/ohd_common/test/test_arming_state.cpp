#include <atomic>
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

#include "openhd_action_handler.h"

int main() {
  openhd::ArmingStateHelper state;
  std::vector<bool> seen;
  // Reproduce telemetry receiving an armed heartbeat before radio discovery.
  state.update_arming_state_if_changed(true);
  state.register_listener("radio", [&](bool armed) { seen.push_back(armed); },
                          true);
  assert((seen == std::vector<bool>{true}));
  state.update_arming_state_if_changed(true);  // Repeated heartbeat.
  assert(seen.size() == 1);
  for (int i = 0; i < 10; ++i) {
    state.update_arming_state_if_changed(false);
    state.update_arming_state_if_changed(true);
  }
  assert(seen.size() == 21);
  for (size_t i = 0; i < seen.size(); ++i) assert(seen[i] == (i % 2 == 0));
  state.unregister_listener("radio");
  state.update_arming_state_if_changed(false);
  assert(seen.size() == 21);
  state.register_listener("radio", [&](bool armed) { seen.push_back(armed); },
                          true);
  assert(!seen.back());  // Disarmed startup is also replayed.
  state.unregister_listener("radio");

  // Existing listeners retain their change-only notification contract.
  int changes = 0;
  state.register_listener("legacy", [&](bool) { ++changes; });
  assert(changes == 0);
  state.update_arming_state_if_changed(true);
  assert(changes == 1);
  state.unregister_listener("legacy");

  // A heartbeat racing registration must not leave the radio with stale state.
  for (int i = 0; i < 200; ++i) {
    state.update_arming_state_if_changed(false);
    std::atomic_bool start{false};
    bool radio_armed = false;
    std::thread heartbeat([&] {
      while (!start.load()) std::this_thread::yield();
      state.update_arming_state_if_changed(true);
    });
    start = true;
    state.register_listener("radio", [&](bool armed) { radio_armed = armed; },
                            true);
    heartbeat.join();
    assert(radio_armed);
    state.unregister_listener("radio");
  }
  std::cout << "Arming startup, transitions and concurrent registration passed\n";
}
