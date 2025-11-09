/******************************************************************************
 * OpenHD
 *
 * Licensed under the GNU General Public License (GPL) Version 3.
 *
 * This software is provided "as-is," without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose, and non-infringement. For details, see the
 * full license in the LICENSE file provided with this source code.
 *
 * Non-Military Use Only:
 * This software and its associated components are explicitly intended for
 * civilian and non-military purposes. Use in any military or defense
 * applications is strictly prohibited unless explicitly and individually
 * licensed otherwise by the OpenHD Team.
 *
 * Contributors:
 * A full list of contributors can be found at the OpenHD GitHub repository:
 * https://github.com/OpenHD
 *
 * © OpenHD, All Rights Reserved.
 ******************************************************************************/

#include "OnboardComputerStatusProvider.h"

#include <set>

#include "onboard_computer_status.hpp"
#include "onboard_computer_status_rpi.hpp"
#include "openhd_spdlog_include.h"
#include "openhd_util_filesystem.h"

// INA219 stuff
constexpr float SHUNT_OHMS = 0.1f;
constexpr float MAX_EXPECTED_AMPS = 3.2f;
constexpr uint16_t RANGE = RANGE_32V;
constexpr uint8_t GAIN = GAIN_8_320MV;
constexpr uint8_t BUS_ADC = ADC_12BIT;
constexpr uint8_t SHUNT_ADC = ADC_12BIT;
// INA219 stuff

static int read_cpu_current_frequency_linux_mhz() {
  static constexpr auto FILEPATH =
      "/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq";
  auto content = OHDFilesystemUtil::opt_read_file(FILEPATH);
  if (!content.has_value()) return -1;
  auto value = OHDUtil::string_to_int(content.value());
  if (!value.has_value()) return -1;
  return value.value() / 1000;
}
int extract_temperature(const std::string& input) {
  auto pos = input.find("temperature:");
  if (pos != std::string::npos) {
    pos += std::string("temperature:").length();
    while (pos < input.size() && std::isspace(input[pos])) {
      ++pos;
    }
    auto end = pos;
    while (end < input.size() && std::isdigit(input[end])) {
      ++end;
    }
    return std::stoi(input.substr(pos, end - pos));
  }
  return 0;
}

static int read_battery_percentage_linux() {
  const std::string filepaths[] = {"/sys/class/power_supply/BAT1/capacity",
                                   "/sys/class/power_supply/BAT0/capacity"};
  for (const auto& filepath : filepaths) {
    if (OHDFilesystemUtil::exists(filepath)) {
      auto content = OHDFilesystemUtil::opt_read_file(filepath);
      if (!content.has_value()) return -2;  // File read error
      auto value = OHDUtil::string_to_int(content.value());
      if (!value.has_value()) return -3;  // Conversion error
      return value.value();
    }
  }
  return -1;
}
static int read_battery_charging_linux() {
  const std::string filepaths[] = {"/sys/class/power_supply/BAT1/status",
                                   "/sys/class/power_supply/BAT0/status"};
  for (const auto& filepath : filepaths) {
    if (OHDFilesystemUtil::exists(filepath)) {
      auto content = OHDFilesystemUtil::opt_read_file(filepath);
      if (!content.has_value()) return -2;  // File read error
      std::string state = content.value();
      int result = -1;  // Default value
      if (state == "Charging\n") {
        result = 1337;
      } else if (state == "Discharging\n") {
        result = 1338;
      } else {
        result = -1;
      }
      return result;  // Returning the charging state
    }
  }
  return -1;  // No battery status file found
}
OnboardComputerStatusProvider::OnboardComputerStatusProvider(bool enable)
    : m_enable(enable), m_ina_219(SHUNT_OHMS, MAX_EXPECTED_AMPS) {
  ina219_log_warning_once(0);
  if (!m_ina_219.has_any_error) {
    m_ina_219.configure(RANGE, GAIN, BUS_ADC, SHUNT_ADC);
  }
  if (m_enable) {
    m_calculate_cpu_usage_thread = std::make_unique<std::thread>(
        &OnboardComputerStatusProvider::calculate_cpu_usage_until_terminate,
        this);
    m_calculate_other_thread = std::make_unique<std::thread>(
        &OnboardComputerStatusProvider::calculate_other_until_terminate, this);
  }
}

OnboardComputerStatusProvider::~OnboardComputerStatusProvider() {
  if (m_enable) {
    terminate = true;
    m_calculate_cpu_usage_thread->join();
    m_calculate_other_thread->join();
  }
}

mavlink_onboard_computer_status_t
OnboardComputerStatusProvider::get_current_status() {
  std::lock_guard<std::mutex> lock(m_curr_onboard_computer_status_mutex);
  return m_curr_onboard_computer_status;
}

void OnboardComputerStatusProvider::calculate_cpu_usage_until_terminate() {
  while (!terminate) {
    const auto before = std::chrono::steady_clock::now();
    const auto value = openhd::onboard::read_cpuload_once_blocking();
    const auto read_time = std::chrono::steady_clock::now() - before;
    if (value.has_value()) {
      // lock mutex and write out
      std::lock_guard<std::mutex> lock(m_curr_onboard_computer_status_mutex);
      m_curr_onboard_computer_status.cpu_cores[0] = value.value();
    }
    // std::cout<<"Took:"<<std::chrono::duration_cast<std::chrono::milliseconds>(read_time).count()<<"\n";
    //  top can block up to X seconds, but in case it doesn't make sure we don't
    //  neccessarily waste cpu here
    const auto minimum_delay = std::chrono::seconds(1);
    if (read_time < minimum_delay) {
      std::this_thread::sleep_for((minimum_delay - read_time));
    }
  }
}

void OnboardComputerStatusProvider::calculate_other_until_terminate() {
  while (!terminate) {
    // We always sleep for 1 second
    // just to make sure to not hog too much cpu here.
    std::this_thread::sleep_for(std::chrono::seconds(1));
    // microhard link
    int microhard_enabled = 21;
    int microhard_rssi = 22;
    int microhard_tx_pwr = 24;
    int microhard_bw = 25;
    int microhard_freq = 26;
    int microhard_tx_rate = 27;
    int microhard_noise = 28;
    int microhard_snr = 29;
    int ohd_encryption = 0;
    // normal stuff
    int8_t curr_temperature_core = 0;
    int8_t curr_temperature_txc0 = 0;
    int8_t curr_temperature_txc1 = 0;
    int txc_temp = 0;
    int curr_clock_cpu = 0;
    int curr_clock_isp = 0;
    int curr_clock_h264 = 0;
    int curr_clock_core = 0;
    int curr_clock_v3d = 0;
    bool curr_rpi_undervolt = false;
    int curr_ina219_voltage = 0;
    int curr_ina219_current = 0;
    const int curr_space_left = OHDFilesystemUtil::get_remaining_space_in_mb();
    const auto ohd_platform =
        static_cast<uint8_t>(OHDPlatform::instance().platform_type);
    const auto curr_ram_usage =
        openhd::onboard::calculate_memory_usage_percent();
    ina219_log_warning_once(curr_ina219_voltage);
    if (!m_ina_219.has_any_error) {
      float voltage = roundf(m_ina_219.voltage() * 1000);
      float current = roundf(m_ina_219.current() * 1000) / 1000;
      curr_ina219_voltage = voltage;
      curr_ina219_current = current;
    } else if (OHDFilesystemUtil::exists(
                   "/sys/class/power_supply/BAT1/capacity") ||
               OHDFilesystemUtil::exists(
                   "/sys/class/power_supply/BAT0/capacity")) {
      curr_ina219_voltage = read_battery_percentage_linux();
      curr_ina219_current = read_battery_charging_linux();
    } else {
      curr_ina219_voltage = -1;
      curr_ina219_current = -1;
    }
    if (OHDFilesystemUtil::exists("/boot/openhd/hidden.txt")) {
      ohd_encryption = 1;
    } else {
      ohd_encryption = 3;
    }
    // Check for presence of rtl88x2eu driver debug interface
    const std::string rtl88x2eu_proc_dir = "/proc/net/rtl88x2eu_ohd/";
    if (OHDFilesystemUtil::exists(rtl88x2eu_proc_dir)) {
      const auto interface_dirs =
          OHDFilesystemUtil::getAllMatchingDirectoriesByPrefix(
              rtl88x2eu_proc_dir, "wl");

      std::set<std::string> seen_ifaces;
      size_t iface_index = 0;

      for (const auto& iface : interface_dirs) {
        if (iface_index > 1) break;

        if (seen_ifaces.find(iface) != seen_ifaces.end()) {
          continue;
        }
        seen_ifaces.insert(iface);

        const std::string thermal_state_file =
            rtl88x2eu_proc_dir + iface + "/thermal_state";

        if (OHDFilesystemUtil::exists(thermal_state_file)) {
          const std::string thermal_content =
              OHDFilesystemUtil::read_file(thermal_state_file);
          int temp = extract_temperature(thermal_content);

          if (temp > 0) {
            if (iface_index == 0) {
              curr_temperature_txc0 = static_cast<int8_t>(temp);
            } else if (iface_index == 1) {
              curr_temperature_txc1 = static_cast<int8_t>(temp);
            }
          }
          ++iface_index;
        }
      }
    }
    if (OHDPlatform::instance().is_rpi()) {
      curr_temperature_core =
          (int8_t)openhd::onboard::rpi::read_temperature_soc_degree();
      // temporary, until we have our own message
      curr_clock_cpu = openhd::onboard::rpi::read_curr_frequency_mhz(
          openhd::onboard::rpi::VCGENCMD_CLOCK_CPU);
      curr_clock_isp = openhd::onboard::rpi::read_curr_frequency_mhz(
          openhd::onboard::rpi::VCGENCMD_CLOCK_ISP);
      curr_clock_h264 = openhd::onboard::rpi::read_curr_frequency_mhz(
          openhd::onboard::rpi::VCGENCMD_CLOCK_H264);
      curr_clock_core = openhd::onboard::rpi::read_curr_frequency_mhz(
          openhd::onboard::rpi::VCGENCMD_CLOCK_CORE);
      curr_clock_v3d = openhd::onboard::rpi::read_curr_frequency_mhz(
          openhd::onboard::rpi::VCGENCMD_CLOCK_V3D);
      curr_rpi_undervolt = openhd::onboard::rpi::vcgencmd_get_undervolt();
    } else {
      const auto cpu_temp = (int8_t)openhd::onboard::readTemperature();
      const auto platform = OHDPlatform::instance();
      curr_temperature_core = cpu_temp;
      curr_temperature_txc0 = txc_temp;
      curr_temperature_txc1 = txc_temp;
      if (platform.is_rock() || platform.platform_type == X_PLATFORM_TYPE_X86) {
        if (OHDFilesystemUtil::exists(
                "/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq")) {
          curr_clock_cpu = read_cpu_current_frequency_linux_mhz();
        }
      }
    }
    {
      // lock mutex and write out
      std::lock_guard<std::mutex> lock(m_curr_onboard_computer_status_mutex);
      m_curr_onboard_computer_status.temperature_core[0] =
          curr_temperature_core;
      m_curr_onboard_computer_status.temperature_core[1] =
          curr_temperature_txc0;
      m_curr_onboard_computer_status.temperature_core[2] =
          curr_temperature_txc1;
      // temporary, until we have our own message
      m_curr_onboard_computer_status.storage_type[0] = curr_clock_cpu;
      m_curr_onboard_computer_status.storage_type[1] = curr_clock_isp;
      m_curr_onboard_computer_status.storage_type[2] = curr_clock_h264;
      m_curr_onboard_computer_status.storage_type[3] = curr_clock_core;
      m_curr_onboard_computer_status.storage_usage[0] = curr_clock_v3d;
      m_curr_onboard_computer_status.storage_usage[1] = curr_space_left;
      m_curr_onboard_computer_status.storage_usage[2] = curr_ina219_voltage;
      m_curr_onboard_computer_status.storage_usage[3] = curr_ina219_current;
      m_curr_onboard_computer_status.link_rx_rate[0] = microhard_enabled;
      m_curr_onboard_computer_status.link_rx_rate[1] = microhard_rssi;
      m_curr_onboard_computer_status.link_rx_rate[2] = microhard_tx_pwr;
      m_curr_onboard_computer_status.link_rx_rate[3] = microhard_bw;
      m_curr_onboard_computer_status.link_rx_rate[4] = microhard_freq;
      m_curr_onboard_computer_status.link_rx_rate[5] = microhard_noise;
      m_curr_onboard_computer_status.link_rx_rate[6] = microhard_snr;
      // openhd status message
      m_curr_onboard_computer_status.link_type[0] =
          ohd_platform;                                 // ohd_platform;
      m_curr_onboard_computer_status.link_type[1] = 0;  // ohd_wifi;
      m_curr_onboard_computer_status.link_type[2] = 0;  // ohd_cam;
      m_curr_onboard_computer_status.link_type[3] = 0;  // ohd_ident;
      m_curr_onboard_computer_status.link_type[4] =
          ohd_encryption;  // ohd_encryption;
      m_curr_onboard_computer_status.ram_usage =
          static_cast<uint32_t>(curr_ram_usage.ram_usage_perc);
      m_curr_onboard_computer_status.ram_total = curr_ram_usage.ram_total_mb;
      m_curr_onboard_computer_status.link_tx_rate[0] =
          curr_rpi_undervolt ? 1 : 0;
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

MavlinkMessage
OnboardComputerStatusProvider::get_current_status_as_mavlink_message(
    const uint8_t sys_id, const uint8_t comp_id,
    const std::optional<ExtraUartInfo>& extra_uart_opt) {
  MavlinkMessage msg;
  auto tmp = get_current_status();
  if (extra_uart_opt.has_value()) {
    const auto& extra_uart = extra_uart_opt.value();
    tmp.fan_speed[0] = extra_uart.fc_sys_id;
    tmp.fan_speed[1] = extra_uart.operating_mode;
  }
  mavlink_msg_onboard_computer_status_encode(sys_id, comp_id, &msg.m, &tmp);
  return msg;
}

void OnboardComputerStatusProvider::ina219_log_warning_once(
    int curr_ina219_voltage) {
  if (!m_ina219_warning_logged && (curr_ina219_voltage > 0)) {
    openhd::log::get_default()->warn("INA219 detected!");
    m_ina219_warning_logged = true;
  }
}
