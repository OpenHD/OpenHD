#include <cassert>
#include <iostream>

#include "openhd_bandwidth_allocation.hpp"
using namespace openhd::bandwidth;
static Consumer camera(unsigned id, unsigned weight) {
  return {id,
          Transport::WiFiBroadcast,
          Priority::Media,
          Control::Dynamic,
          true,
          true,
          4000,
          1000,
          30000,
          weight};
}
int main() {
  Allocator allocator;
  auto a = camera(0, 70), b = camera(1, 30);
  std::vector<Consumer> consumers = {
      a,
      b,
      {10, Transport::WiFiBroadcast, Priority::Essential, Control::Fixed, true,
       true, 100},
      {11, Transport::WiFiBroadcast, Priority::Media, Control::Fixed, true,
       true, 1000}};
  auto result = allocator.calculate(Transport::WiFiBroadcast, 20000, true,
                                    consumers, 1000);
  assert(result.available && !result.overcommitted);
  assert(result.priority_kbits == 125 && result.fixed_kbits == 1150 &&
         result.safety_kbits == 600);
  assert(result.pool_kbits == 18125);
  assert(result.streams[0].percent == 70 && result.streams[1].percent == 30);
  assert(result.streams[0].kbits == 12687 && result.streams[1].kbits == 5437);
  result = allocator.calculate(Transport::WiFiBroadcast, 10000, true, consumers,
                               2000);
  assert(result.streams[0].percent + result.streams[1].percent == 100);
  assert(result.streams[0].kbits < 12687);
  consumers[1].active = false;
  result = allocator.calculate(Transport::WiFiBroadcast, 20000, true, consumers,
                               3000);
  assert(result.streams[0].percent == 100 && !result.streams[1].active);
  consumers[1].active = true;
  consumers[1].control = Control::Fixed;
  result = allocator.calculate(Transport::WiFiBroadcast, 20000, true, consumers,
                               4000);
  assert(result.streams[0].percent == 100 && !result.streams[1].controllable);
  assert(result.fixed_measured_kbits == 5000 && result.fixed_kbits >= 5750);
  consumers[1].measurement_available = false;
  result = allocator.calculate(Transport::WiFiBroadcast, 20000, true, consumers,
                               5000);
  assert(result.waiting && !result.available);
  result =
      allocator.calculate(Transport::WiFiBroadcast, 0, true, consumers, 6000);
  assert(result.pool_kbits == 0 && result.overcommitted);
  consumers[1] = b;
  result = allocator.calculate(Transport::WiFiBroadcast, 20000, true, consumers,
                               7000);
  assert(result.streams[0].percent == 70 && result.streams[1].percent == 30);
  consumers[0].maximum_kbits = 2000;
  result = allocator.calculate(Transport::WiFiBroadcast, 20000, true, consumers,
                               8000);
  assert(result.streams[0].kbits == 2000);
  consumers[2].transport = Transport::Uart;
  Allocator routed;
  result =
      routed.calculate(Transport::WiFiBroadcast, 20000, true, consumers, 1000);
  assert(result.priority_kbits ==
         0);  // UART traffic must not consume WiFi reservations.
  assert(result.streams[0].percent + result.streams[1].percent == 100);
  result =
      routed.calculate(Transport::WiFiBroadcast, 20000, false, consumers, 2000);
  assert(!result.available && result.waiting);
  auto independent = camera(0, 100);
  independent.link_instance = 1;
  auto instance0 = routed.calculate(Transport::WiFiBroadcast, 10000, true,
                                    {independent}, 3000, 0);
  auto instance1 = routed.calculate(Transport::WiFiBroadcast, 10000, true,
                                    {independent}, 3000, 1);
  assert(!instance0.streams[0].active && instance1.streams[0].active &&
         instance1.streams[0].percent == 100);
  auto& store = Store::instance();
  store.publish(Transport::WiFiBroadcast, result);
  store.disable(Transport::WiFiBroadcast);
  assert(!store.read(Transport::WiFiBroadcast).enabled &&
         !store.read(Transport::WiFiBroadcast).available);
  std::cout << "Bandwidth allocation priorities, fixed streams, proportions, "
               "disconnects, constraints, unknown capacity and transport "
               "isolation passed\n";
}
