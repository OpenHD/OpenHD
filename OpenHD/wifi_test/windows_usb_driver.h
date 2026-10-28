#pragma once
#include "wifi_test.h"
namespace openhd::wifi_test {
Json windows_usb_driver_candidates();
// Re-enumerates and validates the exact device instance before installation.
void install_windows_usb_driver(const std::string& instance_id,
                                const std::filesystem::path& directory);
}  // namespace openhd::wifi_test
