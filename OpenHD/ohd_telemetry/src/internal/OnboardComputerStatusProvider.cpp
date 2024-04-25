//
// Created by consti10 on 30.10.22.
//

#include "OnboardComputerStatusProvider.h"

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

OnboardComputerStatusProvider::OnboardComputerStatusProvider(bool enable)
    : m_enable(enable), m_ina_219(SHUNT_OHMS, MAX_EXPECTED_AMPS) {
  ina219_log_warning_once();
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
    int8_t curr_temperature_core = 0;
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
    ina219_log_warning_once();
    if (!m_ina_219.has_any_error) {
      float voltage = roundf(m_ina_219.voltage() * 1000);
      float current = roundf(m_ina_219.current() * 1000) / 1000;
      curr_ina219_voltage = voltage;
      curr_ina219_current = current;
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
      if (platform.is_rock() || platform.platform_type == X_PLATFORM_TYPE_X86) {
        curr_clock_cpu = read_cpu_current_frequency_linux_mhz();
      }
    }
    {
      // lock mutex and write out
      std::lock_guard<std::mutex> lock(m_curr_onboard_computer_status_mutex);
      m_curr_onboard_computer_status.temperature_core[0] =
          curr_temperature_core;
      // temporary, until we have our own message
      m_curr_onboard_computer_status.storage_type[0] = curr_clock_cpu;
      m_curr_onboard_computer_status.storage_type[1] = curr_clock_isp;
      m_curr_onboard_computer_status.storage_type[2] = curr_clock_h264;
      m_curr_onboard_computer_status.storage_type[3] = curr_clock_core;
      m_curr_onboard_computer_status.storage_usage[0] = curr_clock_v3d;
      m_curr_onboard_computer_status.storage_usage[1] = curr_space_left;
      m_curr_onboard_computer_status.storage_usage[2] = curr_ina219_voltage;
      m_curr_onboard_computer_status.storage_usage[3] = curr_ina219_current;
      // openhd status message
      m_curr_onboard_computer_status.link_type[0] =
          ohd_platform;                                 // ohd_platform;
      m_curr_onboard_computer_status.link_type[1] = 0;  // ohd_wifi;
      m_curr_onboard_computer_status.link_type[2] = 0;  // ohd_cam;
      m_curr_onboard_computer_status.link_type[3] = 0;  // ohd_ident;
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

void OnboardComputerStatusProvider::ina219_log_warning_once() {
  if (m_ina_219.has_any_error && !m_ina219_warning_logged) {
    openhd::log::get_default()->warn("INA219 failed - no power monitoring");
    m_ina219_warning_logged = true;
  }
}
