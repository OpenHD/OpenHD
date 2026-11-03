#include <cassert>
#include <iostream>

#include "openhd_bandwidth_allocation.hpp"
#include "openhd_transport_bitrate.hpp"
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
  RateController ethernet;
  assert(ethernet.update(40000, 1000000, 500, 0, 0, 1000) == 40000);
  assert(ethernet.update(40000, 10000, 500, 0, 0, 2000) == 8000);
  assert(ethernet.update(40000, 1000000, 500, 0, 0, 3000) < 40000);
  const int congested = ethernet.update(40000, 1000000, 100000, 100, 0, 4000);
  assert(congested < 8000);
  RateController lte;
  assert(lte.update(40000, 0, UINT32_MAX, UINT32_MAX, 0, 1000) == 1000);
  for (int i=2;i<80;i++) lte.update(40000, 0, 50000, 0, 0, i*1000);
  const int healthy = lte.update(40000, 0, 50000, 0, 0, 80000);
  assert(healthy > 1000 && healthy <= 40000);
  assert(lte.update(40000, 0, 300000, 100, 0, 81000) < healthy);
  RateController handover;
  handover.update(40000, 0, 10000, 0, 0, 0);
  int recovered = 0;
  for (int i=1;i<70;++i) recovered = handover.update(40000, 0, 100000, 0, 0, i*1000);
  assert(recovered > 1000);  // A new stable RTT must not cause permanent backoff.
  BudgetRouter router;
  RelayConstraint relay;
  assert(relay.constrain(40000, 0) == 40000);
  relay.update(5000, 1000);
  assert(relay.constrain(40000, 2000) == 5000);
  assert(relay.constrain(3000, 2000) == 3000);
  assert(relay.constrain(40000, 4001) == 40000);
  router.routed(Transport::WiFiBroadcast, true);
  router.routed(Transport::Ethernet, true);
  assert(router.update(Transport::WiFiBroadcast, 18000)->kbits == 18000);
  assert(router.update(Transport::Ethernet, 40000)->kbits == 18000);
  assert(router.update(Transport::WiFiBroadcast, 8000)->kbits == 8000);
  router.routed(Transport::WiFiBroadcast, false);
  assert(!router.update(Transport::WiFiBroadcast, 9000));
  assert(router.update(Transport::Ethernet, 40000)->kbits == 40000);
  router.routed(Transport::Lte, true);
  assert(router.update(Transport::Lte, 2000)->kbits == 2000);
  Allocator allocator;
  for (auto transport : {Transport::WiFiBroadcast, Transport::Ethernet,
                         Transport::Microhard, Transport::Artosyn, Transport::Lte}) {
    Allocator large_link;
    auto first = camera(0, 70), second = camera(1, 30);
    first.transport = second.transport = transport;
    first.maximum_kbits = second.maximum_kbits = 100000;
    Consumer fixed{12, transport, Priority::Media, Control::Fixed, true, true, 100000};
    auto capped = large_link.calculate(transport, 1000000, true, {first, second, fixed}, 1000);
    assert(capped.capacity_kbits == 1000000 && capped.fixed_kbits == 115000);
    assert(capped.pool_kbits == 50000);
    assert(capped.streams[0].kbits == 35000 && capped.streams[1].kbits == 15000);
    second.active = false;
    capped = large_link.calculate(transport, 1000000, true, {first, second}, 2000);
    assert(capped.streams[0].kbits == 50000);
    first.maximum_kbits = 20000;
    capped = large_link.calculate(transport, 1000000, true, {first}, 3000);
    assert(capped.streams[0].kbits == 20000);
  }
  auto a = camera(0, 70), b = camera(1, 30);
  {
    Allocator pi_pair;
    auto hardware = camera(0, 89), secondary = camera(1, 11);
    hardware.maximum_kbits = 25000;
    secondary.maximum_kbits = 50000;
    auto paired = pi_pair.calculate(Transport::WiFiBroadcast, 1000000, true,
                                    {hardware, secondary}, 1000);
    assert(paired.streams[0].kbits == 25000 && paired.streams[1].kbits == 25000);
    assert(paired.streams[0].percent == 89 && paired.streams[1].percent == 11);
    paired = pi_pair.calculate(Transport::WiFiBroadcast, 5000, true,
                               {hardware, secondary}, 2000);
    assert(!paired.overcommitted);
    assert(paired.streams[0].kbits + paired.streams[1].kbits <= paired.pool_kbits);
    assert(paired.streams[1].kbits >= secondary.minimum_kbits);
    secondary.maximum_kbits = 20000;
    paired = pi_pair.calculate(Transport::WiFiBroadcast, 1000000, true,
                               {hardware, secondary}, 3000);
    // Infeasible capacity stays unused rather than exceeding either encoder.
    for (int t=4000;t<30000;t+=1000)
      paired = pi_pair.calculate(Transport::WiFiBroadcast, 1000000, true,
                                 {hardware, secondary}, t);
    assert(paired.streams[0].kbits == 25000 && paired.streams[1].kbits == 20000);
  }
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
