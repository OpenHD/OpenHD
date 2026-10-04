#include <cassert>

#include "wifi_card_recovery.h"

int main() {
  WiFiCard original{};
  original.device_name = "wlan1";
  original.mac = "11:22:33:44:55:66";
  original.driver_name = "rtl88x2eu_ohd";
  original.type = WiFiCardType::DEVOURER_RTL8822E;
  original.devourer_wb_enabled = true;

  // A cached wlan entry must not trigger kernel setup after USB detach.
  auto stale = original;
  stale.devourer_wb_enabled = false;
  assert(!wifi_card_matches_recovery(original, stale));

  // Live probing can replace a detached netdev with a direct USB interface.
  auto usb = original;
  usb.device_name = "devourer-usb-1-7";
  usb.driver_name = "devourer";
  usb.mac = "00:00:00:00:00:00";
  assert(wifi_card_matches_recovery(original, usb));
  assert(wifi_card_matches_recovery(usb, original));
  auto replugged = usb;
  replugged.device_name = "devourer-usb-1-9";
  assert(wifi_card_matches_recovery(usb, replugged));

  replugged.type = WiFiCardType::DEVOURER_RTL8812A;
  assert(!wifi_card_matches_recovery(original, replugged));
  replugged = usb;
  replugged.devourer_wb_enabled = false;
  assert(!wifi_card_matches_recovery(original, replugged));

  auto other = original;
  other.mac = "aa:bb:cc:dd:ee:ff";
  assert(!wifi_card_matches_recovery(original, other));
  auto renamed = original;
  renamed.device_name = "wlan2";
  assert(wifi_card_matches_recovery(original, renamed));

  // Non-Devourer matching still uses MAC, with the name as a fallback.
  original.devourer_wb_enabled = false;
  renamed.devourer_wb_enabled = false;
  assert(wifi_card_matches_recovery(original, renamed));
  original.mac.clear();
  assert(!wifi_card_matches_recovery(original, renamed));
  assert(wifi_card_matches_recovery(original, original));
}
