#ifndef OPENHD_DEVOURER_USB_DEVICE_PROBE_H
#define OPENHD_DEVOURER_USB_DEVICE_PROBE_H

// OpenHD-owned, read-only USB classification shim. Devourer intentionally
// remains an unmodified radio backend; this mirrors its supported USB IDs and
// silicon-ID read so discovery does not require a kernel network interface.

#include <cstdint>
#include <optional>

#include "AdapterCaps.h"

struct libusb_device;

namespace devourer {

enum class UsbChip : uint8_t {
  Unknown = 0,
  Rtl8812A,
  Rtl8821A,
  Rtl8814A,
  Rtl8821C,
  Rtl8822B,
  Rtl8822C,
  Rtl8822E,
  Rtl8733B,
  Rtl8852B,
  Rtl8852C,
};

struct UsbDeviceProbe {
  uint16_t vid = 0;
  uint16_t pid = 0;
  uint8_t chip_id = 0;
  UsbChip chip = UsbChip::Unknown;
  ChipGeneration generation = ChipGeneration::Unknown;
  const char *chip_name = "unknown";
  const char *marketing_names = "";
  bool supported_by_build = false;
};

// Pure classification entry point, also useful to tests and callers which
// already obtained SYS_CFG2 themselves.
std::optional<UsbDeviceProbe> classify_usb_device(uint16_t vid, uint16_t pid,
                                                  uint8_t chip_id);

// Returns nullopt for USB IDs outside Devourer's device set. For a known USB
// ID which cannot currently be opened/read, returns an Unknown probe so callers
// can keep it out of a radio role without guessing its silicon from the PID.
std::optional<UsbDeviceProbe> probe_usb_device(libusb_device *device);

}  // namespace devourer

#endif
