#pragma once
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <map>
#include <mutex>
#include <optional>
#include <vector>

namespace openhd::bandwidth {
enum class Transport : uint8_t {
  Unknown = 0,
  WiFiBroadcast = 1,
  Ethernet = 2,
  Uart = 3
};
// A link instance identifies one physical/logical transport. Separate WiFi
// radios can share a transport type without sharing a capacity or history.
struct Link {
  Transport transport = Transport::Unknown;
  unsigned instance = 0;
  bool operator<(const Link& other) const {
    return transport < other.transport ||
           (transport == other.transport && instance < other.instance);
  }
};
enum class Priority : uint8_t { Essential, Media };
enum class Control : uint8_t { Fixed, Dynamic };
struct Consumer {
  unsigned id = 0;
  Transport transport = Transport::Unknown;
  Priority priority = Priority::Media;
  Control control = Control::Fixed;
  bool active = false;
  bool measurement_available = false;
  int measured_kbits = 0;
  int minimum_kbits = 1000;
  int maximum_kbits = 100000;
  unsigned weight = 50;
  unsigned link_instance = 0;
};
struct StreamBudget {
  bool active = false, controllable = false;
  unsigned percent = 0;
  int kbits = 0;
};
struct Snapshot {
  bool enabled = false, available = false, waiting = false,
       overcommitted = false;
  int capacity_kbits = 0, priority_kbits = 0, fixed_kbits = 0, safety_kbits = 0;
  int pool_kbits = 0, fixed_measured_kbits = 0;
  std::array<StreamBudget, 2> streams{};
};
class Store {
 public:
  static Store& instance() {
    static Store store;
    return store;
  }
  void publish(Transport link, Snapshot value, unsigned instance = 0) {
    std::lock_guard<std::mutex> lock(mutex);
    values[{link, instance}] = value;
    updated[{link, instance}] = std::chrono::steady_clock::now();
  }
  Snapshot read(Transport link, unsigned instance = 0) {
    std::lock_guard<std::mutex> lock(mutex);
    auto value = values[{link, instance}];
    if (value.enabled &&
        std::chrono::steady_clock::now() - updated[{link, instance}] >
            std::chrono::seconds(3)) {
      value.available = false;
      value.waiting = true;
    }
    return value;
  }
  void disable(Transport link, unsigned instance = 0) {
    std::lock_guard<std::mutex> lock(mutex);
    values[{link, instance}].enabled = false;
    values[{link, instance}].available = false;
  }

 private:
  std::mutex mutex;
  std::map<Link, Snapshot> values;
  std::map<Link, std::chrono::steady_clock::time_point> updated;
};
// Payload-capacity allocator. Each transport has its own history and consumers;
// capacities from independent transports are never added together.
class Allocator {
 public:
  Snapshot calculate(Transport link, int usable_kbits,
                     bool measurements_available,
                     const std::vector<Consumer>& consumers, int64_t now_ms,
                     unsigned instance = 0) {
    auto& state = states[{link, instance}];
    Snapshot out;
    out.enabled = true;
    out.capacity_kbits = std::max(0, usable_kbits);
    out.available = usable_kbits > 0 && measurements_available;
    out.waiting = !out.available;
    int priority = 0, fixed = 0;
    unsigned weight = 0, count = 0;
    for (const auto& consumer : consumers) {
      if (consumer.transport != link || consumer.link_instance != instance ||
          !consumer.active)
        continue;
      if (consumer.priority == Priority::Essential)
        priority += std::max(0, consumer.measured_kbits);
      else if (consumer.control == Control::Fixed) {
        fixed += std::max(0, consumer.measured_kbits);
        if (!consumer.measurement_available) {
          out.available = false;
          out.waiting = true;
        }
      } else if (consumer.id < 2) {
        ++count;
        weight += consumer.weight;
      }
      if (consumer.id < 2) {
        out.streams[consumer.id].active = true;
        out.streams[consumer.id].controllable =
            consumer.control == Control::Dynamic;
      }
    }
    out.fixed_measured_kbits = fixed;
    // Fast attack, slow release for reservations. These are estimates, not
    // measured traffic.
    state.priority =
        std::max(double(priority), state.priority * 0.85 + priority * 0.15);
    state.fixed = std::max(double(fixed), state.fixed * 0.90 + fixed * 0.10);
    out.priority_kbits = int(state.priority * 1.25 + 0.5);
    out.fixed_kbits = int(state.fixed * 1.15 + 0.5);
    // Management/framing allowance stays local to this link, even without
    // MAVLink.
    out.safety_kbits = std::max(64, out.capacity_kbits * 3 / 100);
    const int required =
        out.priority_kbits + out.fixed_kbits + out.safety_kbits;
    out.pool_kbits = std::max(0, out.capacity_kbits - required);
    out.overcommitted = required > out.capacity_kbits;
    unsigned assigned = 0, remaining = count;
    for (const auto& consumer : consumers) {
      if (consumer.transport != link || consumer.link_instance != instance ||
          !consumer.active || consumer.control != Control::Dynamic ||
          consumer.id >= 2)
        continue;
      auto& budget = out.streams[consumer.id];
      --remaining;
      budget.percent = remaining == 0 ? 100 - assigned
                                      : (weight ? consumer.weight * 100 / weight
                                                : 100 / count);
      assigned += budget.percent;
      const int maximum =
          std::max(consumer.minimum_kbits, consumer.maximum_kbits);
      const int target = std::clamp(out.pool_kbits * int(budget.percent) / 100,
                                    consumer.minimum_kbits, maximum);
      // Bound upward movement and suppress tiny oscillations. Capacity
      // reductions apply immediately so smoothing cannot keep an unsafe
      // previous target.
      int applied = state.last[consumer.id];
      if (applied <= 0 || target < applied ||
          state.percent[consumer.id] != budget.percent)
        applied = target;
      else if (now_ms - state.updated[consumer.id] >= 1000 &&
               target - applied >= std::max(100, applied / 20))
        applied = std::min(target, applied + std::max(100, applied / 5));
      applied = std::clamp(applied, consumer.minimum_kbits, maximum);
      budget.kbits = applied;
      state.last[consumer.id] = applied;
      state.percent[consumer.id] = budget.percent;
      if (applied != state.previous[consumer.id])
        state.updated[consumer.id] = now_ms;
      state.previous[consumer.id] = applied;
    }
    int allocated = 0;
    for (const auto& budget : out.streams)
      if (budget.controllable && budget.active) allocated += budget.kbits;
    out.overcommitted = out.overcommitted || allocated > out.pool_kbits;
    return out;
  }

 private:
  struct State {
    double priority = 0, fixed = 0;
    std::array<int, 2> last{}, previous{};
    std::array<unsigned, 2> percent{};
    std::array<int64_t, 2> updated{};
  };
  std::map<Link, State> states;
};
}  // namespace openhd::bandwidth
