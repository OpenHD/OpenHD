#pragma once

#include "wifi_card.h"

// Devourer may detach the kernel netdev, so a live USB pseudo-interface can
// replace the original wlan name. Never accept cached kernel metadata as a
// replacement for a radio admitted by the USB probe.
inline bool wifi_card_matches_recovery(const WiFiCard& expected,
                                      const WiFiCard& candidate) {
  if (expected.devourer_wb_enabled != candidate.devourer_wb_enabled) {
    return false;
  }
  if (expected.devourer_wb_enabled) {
    if (candidate.type != expected.type) return false;
    if (expected.driver_name == "devourer" ||
        candidate.driver_name == "devourer") {
      return true;
    }
  }
  if (!expected.mac.empty()) return candidate.mac == expected.mac;
  return candidate.device_name == expected.device_name;
}
