#include "devourer_usb_device_probe.h"

#include <libusb.h>

#include "UsbTransport.h"
#include "kestrel/KestrelUsbIds.h"
#include "rtl8733b/Rtl8733bUsbIds.h"

namespace devourer {
namespace {

bool is_known_jaguar_usb_id(const uint16_t vid, const uint16_t pid) {
  const uint32_t id = (static_cast<uint32_t>(vid) << 16) | pid;
  switch (id) {
    case 0x0bda8812:
    case 0x0bda0811:
    case 0x0bdaa811:
    case 0x0bdab811:
    case 0x0bda8813:
    case 0x0bdab812:
    case 0x0bdab82c:
    case 0x0bda0820:
    case 0x0bda0821:
    case 0x0bda0823:
    case 0x0bda8822:
    case 0x0bdac811:
    case 0x0bdac812:
    case 0x0bdac82c:
    case 0x0bdac82e:
    case 0x0bda881a:
    case 0x0bda881b:
    case 0x0bda881c:
    case 0x0bdaa81a:
    case 0x0bdae822:
    case 0x0bdaa82a:
    case 0x2357011e:
    case 0x23570120:
    case 0x23570122:
    case 0x2357012d:
    case 0x04110242:
    case 0x0411029b:
    case 0x04bb0953:
    case 0x056e4007:
    case 0x056e400e:
    case 0x056e400f:
    case 0x08469052:
    case 0x0e660023:
    case 0x20013314:
    case 0x20013318:
    case 0x2019ab32:
    case 0x20f4804b:
    case 0x38236249:
    case 0x7392a811:
    case 0x7392a812:
    case 0x7392a813:
    case 0x7392b611:
      return true;
    default:
      return false;
  }
}

bool chip_is_enabled(const UsbChip chip) {
  switch (chip) {
    case UsbChip::Rtl8812A:
    case UsbChip::Rtl8821A:
#if defined(DEVOURER_HAVE_JAGUAR1)
      return true;
#else
      return false;
#endif
    case UsbChip::Rtl8814A:
#if defined(DEVOURER_HAVE_8814)
      return true;
#else
      return false;
#endif
    case UsbChip::Rtl8822B:
#if defined(DEVOURER_HAVE_JAGUAR2_8822B)
      return true;
#else
      return false;
#endif
    case UsbChip::Rtl8821C:
#if defined(DEVOURER_HAVE_JAGUAR2_8821C)
      return true;
#else
      return false;
#endif
    case UsbChip::Rtl8822C:
#if defined(DEVOURER_HAVE_JAGUAR3_8822C)
      return true;
#else
      return false;
#endif
    case UsbChip::Rtl8822E:
#if defined(DEVOURER_HAVE_JAGUAR3_8822E)
      return true;
#else
      return false;
#endif
    case UsbChip::Rtl8733B:
#if defined(DEVOURER_HAVE_8733B)
      return true;
#else
      return false;
#endif
    case UsbChip::Rtl8852B:
#if defined(DEVOURER_HAVE_KESTREL_8852B)
      return true;
#else
      return false;
#endif
    case UsbChip::Rtl8852C:
#if defined(DEVOURER_HAVE_KESTREL_8852C)
      return true;
#else
      return false;
#endif
    default:
      return false;
  }
}

UsbDeviceProbe make_probe(const uint16_t vid, const uint16_t pid,
                          const uint8_t chip_id, const UsbChip chip,
                          const ChipGeneration generation,
                          const char *chip_name,
                          const char *marketing_names) {
  return UsbDeviceProbe{vid, pid, chip_id, chip, generation, chip_name,
                        marketing_names, chip_is_enabled(chip)};
}

}  // namespace

std::optional<UsbDeviceProbe> classify_usb_device(const uint16_t vid,
                                                  const uint16_t pid,
                                                  const uint8_t chip_id) {
  if (const auto variant = kestrel::variant_for_usb_id(vid, pid)) {
    if (*variant == kestrel::ChipVariant::C8852B) {
      return make_probe(vid, pid, chip_id, UsbChip::Rtl8852B,
                        ChipGeneration::Kestrel, "RTL8852B",
                        "RTL8852BU/RTL8832BU");
    }
    return make_probe(vid, pid, chip_id, UsbChip::Rtl8852C,
                      ChipGeneration::Kestrel, "RTL8852C",
                      "RTL8852CU/RTL8832CU");
  }
  const bool known_8733 = rtl8733b::is_usb_id(vid, pid);
  if (!known_8733 && !is_known_jaguar_usb_id(vid, pid)) return std::nullopt;
  switch (chip_id) {
    case 0x04:
      return make_probe(vid, pid, chip_id, UsbChip::Rtl8812A,
                        ChipGeneration::Jaguar1, "RTL8812A",
                        "RTL8812AU/RTL8811AU");
    case 0x05:
      return make_probe(vid, pid, chip_id, UsbChip::Rtl8821A,
                        ChipGeneration::Jaguar1, "RTL8821A", "RTL8821AU");
    case 0x08:
      return make_probe(vid, pid, chip_id, UsbChip::Rtl8814A,
                        ChipGeneration::Jaguar1, "RTL8814A", "RTL8814AU");
    case 0x09:
      return make_probe(vid, pid, chip_id, UsbChip::Rtl8821C,
                        ChipGeneration::Jaguar2, "RTL8821C",
                        "RTL8811CU/RTL8821CU");
    case 0x0a:
    case 0x50:
      return make_probe(vid, pid, chip_id, UsbChip::Rtl8822B,
                        ChipGeneration::Jaguar2, "RTL8822B", "RTL8822BU");
    case 0x13:
      return make_probe(vid, pid, chip_id, UsbChip::Rtl8822C,
                        ChipGeneration::Jaguar3, "RTL8822C",
                        "RTL8812CU/RTL8822CU");
    case 0x17:
      return make_probe(vid, pid, chip_id, UsbChip::Rtl8822E,
                        ChipGeneration::Jaguar3, "RTL8822E",
                        "RTL8812EU/RTL8822EU");
    case rtl8733b::kChipId:
      return make_probe(vid, pid, chip_id, UsbChip::Rtl8733B,
                        ChipGeneration::Rtl8733b, "RTL8733B",
                        "RTL8731BU/RTL8733BU");
    default:
      return UsbDeviceProbe{vid, pid, chip_id};
  }
}

std::optional<UsbDeviceProbe> probe_usb_device(libusb_device *device) {
  if (!device) return std::nullopt;
  libusb_device_descriptor descriptor{};
  if (libusb_get_device_descriptor(device, &descriptor) != 0) {
    return std::nullopt;
  }
  const auto known = classify_usb_device(descriptor.idVendor,
                                         descriptor.idProduct, 0);
  if (!known) return std::nullopt;
  if (known->chip != UsbChip::Unknown) return known;  // Kestrel is PID-gated.

  libusb_device_handle *handle = nullptr;
  if (libusb_open(device, &handle) != 0 || !handle) return known;
  uint8_t chip_id = 0;
  for (int attempt = 0; attempt < 4; ++attempt) {
    const int rc = libusb_control_transfer(
        handle, REALTEK_USB_VENQT_READ, 5, 0x00FC, 0, &chip_id,
        sizeof(chip_id), USB_TIMEOUT);
    if (rc == sizeof(chip_id) && chip_id != 0) break;
  }
  libusb_close(handle);
  return classify_usb_device(descriptor.idVendor, descriptor.idProduct,
                             chip_id);
}

}  // namespace devourer
