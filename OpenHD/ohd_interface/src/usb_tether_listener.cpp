//
// Created by consti10 on 21.05.22.
//

#include "usb_tether_listener.h"

#include <arpa/inet.h>

#include <cassert>
#include <utility>

#include "openhd_spdlog.h"
#include "openhd_spdlog_include.h"

USBTetherListener::USBTetherListener() {
  m_console = openhd::log::create_or_get("usb_listener");
  assert(m_console);
  m_check_connection_thread_stop = false;
  m_check_connection_thread =
      std::make_unique<std::thread>([this]() { loopInfinite(); });
}

USBTetherListener::~USBTetherListener() {
  m_check_connection_thread_stop = true;
  if (m_check_connection_thread->joinable()) {
    m_check_connection_thread->join();
  }
  m_check_connection_thread.reset();
}

void USBTetherListener::loopInfinite() {
  while (!m_check_connection_thread_stop) {
    connectOnce();
  }
}

static std::vector<std::string> get_usb_tethering_devices() {
  const auto net_devices =
      OHDFilesystemUtil::getAllEntriesFilenameOnlyInDirectory(
          "/sys/class/net/");
  std::vector<std::string> ret;
  for (const auto& net_device : net_devices) {
    const auto opt_file_device_uevent = OHDFilesystemUtil::opt_read_file(
        fmt::format("/sys/class/net/{}/device/uevent", net_device), false);
    if (opt_file_device_uevent.has_value() &&
        OHDUtil::contains(opt_file_device_uevent.value(),
                          "DRIVER=rndis_host")) {
      ret.push_back(net_device);
    }
  }
  return ret;
}

void USBTetherListener::connectOnce() {
  m_console->debug("connectOnce()");
  const std::string connected_devices_directory = "/sys/class/net/";
  std::string connected_device_name;
  while (!m_check_connection_thread_stop) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    const auto usb_tether_devices = get_usb_tethering_devices();
    if (!usb_tether_devices.empty()) {
      m_console->debug("Found {} tethering devices",
                       OHDUtil::str_vec_as_string(usb_tether_devices));
      connected_device_name = usb_tether_devices.at(0);
      break;
    }
  }
  // We were stopped externally, no reason to continue
  if (connected_device_name.empty()) return;
  m_console->info("Found USB tethering device {}", connected_device_name);
  // now we find the IP of the connected device so we can forward video and more
  // to it. example on my Ubuntu pc: ip route list dev usb0 default via
  // 192.168.18.229 proto dhcp metric 101 192.168.18.0/24 proto kernel scope
  // link src 192.168.18.155 metric 101
  const auto run_command_result_opt = OHDUtil::run_command_out(
      fmt::format("ip route list dev {}", connected_device_name));
  if (run_command_result_opt == std::nullopt) {
    m_console->warn("run command out no result");
    return;
  }
  const auto& run_command_result = run_command_result_opt.value();
  const auto ip_external_device =
      OHDUtil::string_in_between("default via ", " proto", run_command_result);
  // const auto ip_external_device2= OHDUtil::string_in_between("default via ","
  // proto",run_command_result);
  const auto ip_self_network =
      OHDUtil::string_in_between("src ", " metric", run_command_result);

  const auto external_device =
      openhd::ExternalDevice{connected_device_name, ip_external_device};
  // Check if both are valid IPs (otherwise, perhaps the parsing got fucked up)
  if (!external_device.is_valid()) {
    m_console->warn("{} not valid", external_device.to_string());
    std::this_thread::sleep_for(std::chrono::seconds(2));  // try again later
    return;
  }
  m_console->info("found device:{}", external_device.to_string());
  openhd::ExternalDeviceManager::instance().on_new_external_device(
      external_device, true);
  // check in regular intervals if the tethering device disconnects.
  while (!m_check_connection_thread_stop) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (!OHDFilesystemUtil::exists(connected_devices_directory +
                                   connected_device_name)) {
      m_console->warn("USB Tether device {} disconnected",
                      connected_device_name);
      break;
    }
  }
  openhd::ExternalDeviceManager::instance().on_new_external_device(
      external_device, false);
}
