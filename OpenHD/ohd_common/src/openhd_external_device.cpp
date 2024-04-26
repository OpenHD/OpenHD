//
// Created by consti10 on 09.01.24.
//

#include "openhd_external_device.h"

#include "openhd_config.h"
#include "openhd_settings_directories.h"
#include "openhd_spdlog.h"
#include "openhd_util.h"

openhd::ExternalDeviceManager::ExternalDeviceManager() {
  // Here one can manually declare any IP addresses openhd should forward video
  // / telemetry to
  const auto config = openhd::load_config();
  for (const auto& ip : config.NW_MANUAL_FORWARDING_IPS) {
    if (OHDUtil::is_valid_ip(ip)) {
      m_manual_ips.push_back(ip);
    } else {
      openhd::log::get_default()->warn("[{}] is not a valid ip", ip);
    }
  }
  for (const auto& ip : m_manual_ips) {
    on_new_external_device(ExternalDevice{"manual", ip}, true);
  }
}

openhd::ExternalDeviceManager::~ExternalDeviceManager() {
  for (const auto& ip : m_manual_ips) {
    on_new_external_device(ExternalDevice{"manual", ip}, false);
  }
}

openhd::ExternalDeviceManager& openhd::ExternalDeviceManager::instance() {
  static openhd::ExternalDeviceManager instance;
  return instance;
}

void openhd::ExternalDeviceManager::on_new_external_device(
    const openhd::ExternalDevice& external_device, bool connected) {
  std::lock_guard<std::mutex> guard(m_ext_devices_lock);
  if (m_remove_all_called) return;
  openhd::log::get_default()->debug("Got {} {}", external_device.to_string(),
                                    connected);
  const auto id = external_device.create_identifier();
  if (connected) {
    if (m_curr_ext_devices.find(id) != m_curr_ext_devices.end()) {
      openhd::log::get_default()->warn("Device {} already exists",
                                       external_device.to_string());
      return;
    }
    // New external device connected
    // log such that the message is shown in QOpenHD
    openhd::log::log_via_mavlink(5, "External device connected");
    m_curr_ext_devices[id] = external_device;
    m_external_device_count++;
    for (auto& cb : m_callbacks) {
      cb(external_device, true);
    }
  } else {
    if (m_curr_ext_devices.find(id) == m_curr_ext_devices.end()) {
      openhd::log::get_default()->warn("Device {} does not exist",
                                       external_device.to_string());
      return;
    }
    // warning in QOpenHD
    openhd::log::get_default()->warn("External device disconnected");
    // existing external device disconnected
    m_curr_ext_devices.erase(id);
    m_external_device_count--;
    for (auto& cb : m_callbacks) {
      cb(external_device, false);
    }
  }
}

void openhd::ExternalDeviceManager::register_listener(
    openhd::EXTERNAL_DEVICE_CALLBACK cb) {
  std::lock_guard<std::mutex> guard(m_ext_devices_lock);
  // Notify the callback to register of any already connected devices
  for (auto& [id, device] : m_curr_ext_devices) {
    cb(device, true);
  }
  m_callbacks.push_back(cb);
}

void openhd::ExternalDeviceManager::remove_all() {
  auto console = openhd::log::create_or_get("ExternalDeviceManager");
  console->debug("removing all devices - begin");
  std::lock_guard<std::mutex> guard(m_ext_devices_lock);
  for (auto& device_and_ip : m_curr_ext_devices) {
    auto external_device = device_and_ip.second;
    console->debug("Removing {}", external_device.to_string());
    for (auto& cb : m_callbacks) {
      cb(external_device, false);
    }
  }
  m_curr_ext_devices.clear();
  m_external_device_count = 0;
  m_remove_all_called = true;
  console->debug("removing all devices - end");
}

uint8_t openhd::ExternalDeviceManager::get_external_device_count() {
  return m_external_device_count;
}

std::string openhd::ExternalDevice::to_string() const {
  // return fmt::format("ExternalDevice {} [local:[{}]
  // remote:[{}]]",tag,local_network_ip,external_device_ip);
  return fmt::format("ExternalDevice {} remote:[{}]]", tag, external_device_ip);
}

bool openhd::ExternalDevice::is_valid() const {
  // return OHDUtil::is_valid_ip(local_network_ip) &&
  // OHDUtil::is_valid_ip(external_device_ip);
  return OHDUtil::is_valid_ip(external_device_ip);
}

std::string openhd::ExternalDevice::create_identifier() const {
  assert(is_valid());
  // return local_network_ip + "_" + external_device_ip;
  return external_device_ip;
}
