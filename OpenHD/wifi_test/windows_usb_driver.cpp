#include "windows_usb_driver.h"

#include <libwdi.h>

#include <cstring>
#include <stdexcept>

#include "devourer_usb_device_probe.h"
namespace openhd::wifi_test {
namespace {
void check(int result, const char* stage) {
  if (result != WDI_SUCCESS)
    throw std::runtime_error(std::string(stage) + ": " + wdi_strerror(result));
}
struct Devices {
  wdi_device_info* list = nullptr;
  Devices() {
    wdi_options_create_list options{};
    options.list_all = TRUE;
    options.trim_whitespaces = TRUE;
    check(wdi_create_list(&list, &options), "USB driver discovery");
  }
  ~Devices() { wdi_destroy_list(list); }
};
bool usable_driver(const char* driver) {
  return driver &&
         (_stricmp(driver, "WinUSB") == 0 || _stricmp(driver, "libusbK") == 0 ||
          _stricmp(driver, "libusb0") == 0);
}
}  // namespace
Json windows_usb_driver_candidates() {
  Devices devices;
  auto result = Json::array();
  for (auto* d = devices.list; d; d = d->next) {
    if (!d->device_id || usable_driver(d->driver)) continue;
    auto identity = describe_usb_setup_candidate(d->vid, d->pid);
    if (!identity) continue;
    std::string label = identity->at("label");
    if (identity->at("model") == "" && d->desc)
      label = std::string(d->desc) + " | " + label;
    identity->update(
        {{"vid", d->vid},
         {"pid", d->pid},
         {"instance_id", d->device_id},
         {"driver", d->driver ? d->driver : "No driver"},
         {"selector", std::string("windows-driver:") + d->device_id},
         {"label", label + " | WinUSB setup required"}});
    result.push_back(std::move(*identity));
  }
  return result;
}
void install_windows_usb_driver(const std::string& instance_id,
                                const std::filesystem::path& directory) {
  Devices devices;
  wdi_device_info* selected = nullptr;
  for (auto* d = devices.list; d; d = d->next)
    if (d->device_id && instance_id == d->device_id) selected = d;
  if (!selected ||
      !devourer::is_usb_driver_setup_candidate(selected->vid, selected->pid))
    throw std::runtime_error(
        "The selected supported USB adapter is no longer connected");
  if (!wdi_is_driver_supported(WDI_WINUSB, nullptr))
    throw std::runtime_error(
        "WinUSB installation support is missing from this build");
  std::filesystem::create_directories(directory);
  const auto utf8 = directory.u8string();
  const std::string path(reinterpret_cast<const char*>(utf8.data()),
                         utf8.size());
  wdi_options_prepare_driver prepare{};
  prepare.driver_type = WDI_WINUSB;
  char vendor[] = "OpenHD";
  prepare.vendor_name = vendor;
  check(
      wdi_prepare_driver(selected, path.c_str(), "openhd-winusb.inf", &prepare),
      "Prepare WinUSB driver");
  wdi_options_install_driver install{};
  install.pending_install_timeout = 120000;
  check(
      wdi_install_driver(selected, path.c_str(), "openhd-winusb.inf", &install),
      "Install WinUSB driver");
}
}  // namespace openhd::wifi_test
