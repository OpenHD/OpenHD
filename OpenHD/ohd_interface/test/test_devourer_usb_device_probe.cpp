#include <cassert>

#include "devourer_usb_device_probe.h"

int main() {
  // A common RTL8812AU rebrand must reach the silicon-ID probe, but its USB
  // identity alone must never be enough to mark it Devourer-capable.
  const auto unidentified = devourer::classify_usb_device(0x2357, 0x0101, 0);
  assert(unidentified);
  assert(unidentified->chip == devourer::UsbChip::Unknown);

  const auto rtl8812a = devourer::classify_usb_device(0x2357, 0x0101, 0x04);
  assert(rtl8812a);
  assert(rtl8812a->chip == devourer::UsbChip::Rtl8812A);

  // The Jaguar factory can handle previously unseen Realtek PIDs and OEM
  // rebrands identified by a Realtek kernel netdev; neither is admitted on
  // the VID:PID alone.
  assert(devourer::classify_usb_device(0x0bda, 0x1234, 0)->chip ==
         devourer::UsbChip::Unknown);
  assert(devourer::classify_usb_device(0x0bda, 0x1234, 0x04)->chip ==
         devourer::UsbChip::Rtl8812A);
  assert(!devourer::classify_usb_device(0x1234, 0x5678, 0x04));
  assert(devourer::classify_usb_device(0x1234, 0x5678, 0x04, true)->chip ==
         devourer::UsbChip::Rtl8812A);

}
