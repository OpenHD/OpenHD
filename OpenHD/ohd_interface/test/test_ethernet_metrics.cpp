#include <cassert>
#include <iostream>
#include <memory>

#include "ethernet_peer_probe.h"
#include "openhd_link_usage.hpp"

int main() {
  using namespace openhd::ethernet;
  assert(rate(1000000, 500, 8000) == 16000000);
  assert(rate(10, 0, 8000) == 0);
  assert(rate(UINT64_MAX, 1, 8000) == UINT32_MAX);
  ProbeWindow window;
  Metrics metrics;
  window.complete(false);
  window.apply(metrics, true);
  assert(metrics.rtt_us == kUnknown && metrics.probe_loss_permille == kUnknown);
  window.complete(true, 1000);
  window.complete(true, 3000);
  window.apply(metrics, true);
  assert(metrics.rtt_us == 3000 && metrics.jitter_us == 500);
  assert(metrics.probe_loss_permille == 333);
  metrics = {};
  window.apply(metrics, false);
  assert(metrics.rtt_us == kUnknown && metrics.probe_loss_permille == 333);
  for (int i = 0; i < 60; ++i) window.complete(true, 1000);
  window.apply(metrics, true);
  assert(metrics.probe_loss_permille == 0);
  window.reset();
  metrics = {};
  window.apply(metrics, true);
  assert(metrics.probe_loss_permille == kUnknown);

  auto& registry = openhd::link_usage::Registry::instance();
  registry.add_link("ETHERNET");
  Metrics transport;
  transport.tx_bps = 12300000;
  transport.rx_bps = 64000;
  registry.set_ethernet_metrics(1000000000, transport);
  std::this_thread::sleep_for(std::chrono::milliseconds(120));
  auto samples = registry.sample();
  assert(samples.size() == 1);
  assert(samples[0].has_ethernet_metrics &&
         samples[0].capacity_bps == 1000000000);
  assert(samples[0].ethernet_metrics.tx_bps == 12300000);
  registry.remove_link("ETHERNET");
  registry.set_ethernet_metrics(1000000000, transport);
  assert(registry.sample().empty());  // Never resurrect a removed transport.

  auto ground = std::make_unique<EthernetPeerProbe>(
      false, [] { return "127.0.0.1"; }, 49993);
  EthernetPeerProbe air(
      true, [] { return "127.0.0.1"; }, 49993);
  bool replied = false;
  for (int i = 0; i < 80; ++i) {
    metrics = {};
    air.apply(metrics);
    if (metrics.rtt_us != kUnknown) {
      replied = true;
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
  assert(replied && metrics.probe_loss_permille < 1000);
  ground.reset();
  std::this_thread::sleep_for(std::chrono::seconds(5));
  metrics = {};
  air.apply(metrics);
  assert(metrics.rtt_us == kUnknown && metrics.probe_loss_permille > 0);
  std::cout
      << "Ethernet rates, window, registry, UDP probe and outage passed\n";
}
