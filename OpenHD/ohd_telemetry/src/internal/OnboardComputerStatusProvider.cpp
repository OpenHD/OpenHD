//
// Created by consti10 on 30.10.22.
//

#include "OnboardComputerStatusProvider.h"

#include "OnboardComputerStatus.hpp"
#include "openhd_util_filesystem.h"

//INA219 stuff
constexpr float SHUNT_OHMS = 0.1f;
constexpr float MAX_EXPECTED_AMPS = 3.2f;
constexpr uint16_t RANGE = RANGE_16V;
constexpr uint8_t GAIN = GAIN_8_320MV;
constexpr uint8_t BUS_ADC = ADC_12BIT;
constexpr uint8_t SHUNT_ADC = ADC_12BIT;
//INA219 stuff

OnboardComputerStatusProvider::OnboardComputerStatusProvider(OHDPlatform platform)
    : m_platform(platform),
      m_ina_219(SHUNT_OHMS, MAX_EXPECTED_AMPS)
{
  ina219_log_warning_once();
  if(!m_ina_219.has_any_error){
    m_ina_219.configure(RANGE, GAIN, BUS_ADC, SHUNT_ADC);
  }
  m_calculate_cpu_usage_thread=std::make_unique<std::thread>(&OnboardComputerStatusProvider::calculate_cpu_usage_until_terminate, this);
  m_calculate_other_thread=std::make_unique<std::thread>(&OnboardComputerStatusProvider::calculate_other_until_terminate, this);
}

OnboardComputerStatusProvider::~OnboardComputerStatusProvider() {
  terminate= true;
  m_calculate_cpu_usage_thread->join();
  m_calculate_other_thread->join();
}

mavlink_onboard_computer_status_t OnboardComputerStatusProvider::get_current_status() {
  std::lock_guard<std::mutex> lock(m_curr_onboard_computer_status_mutex);
  return m_curr_onboard_computer_status;
}

void OnboardComputerStatusProvider::calculate_cpu_usage_until_terminate() {
  while (!terminate){
    const auto before=std::chrono::steady_clock::now();
    const auto value=OnboardComputerStatus::read_cpuload_once_blocking();
    const auto read_time=std::chrono::steady_clock::now()-before;
    if(value.has_value()){
      // lock mutex and write out
      std::lock_guard<std::mutex> lock(m_curr_onboard_computer_status_mutex);
      m_curr_onboard_computer_status.cpu_cores[0]=value.value();
    }
    //std::cout<<"Took:"<<std::chrono::duration_cast<std::chrono::milliseconds>(read_time).count()<<"\n";
    // top can block up to X seconds, but in case it doesn't make sure we don't neccessarily waste cpu here
    const auto minimum_delay=std::chrono::seconds(1);
    if(read_time<minimum_delay){
      std::this_thread::sleep_for((minimum_delay-read_time));
    }
  }
}

void OnboardComputerStatusProvider::calculate_other_until_terminate() {
  while (!terminate){
    // We always sleep for 1 second
    // just to make sure to not hog too much cpu here.
    std::this_thread::sleep_for(std::chrono::seconds(1));
    int8_t curr_temperature_core=0;
    int curr_clock_cpu=0;
    int curr_clock_isp=0;
    int curr_clock_h264=0;
    int curr_clock_core=0;
    int curr_clock_v3d=0;
    int curr_ina219_voltage=0;
    int curr_ina219_current=0;

    const int curr_space_left=OHDFilesystemUtil::get_remaining_space_in_mb();
    const auto curr_ram_usage=OnboardComputerStatus::calculate_memory_usage_percent();
    ina219_log_warning_once();
    if(!m_ina_219.has_any_error){
      float voltage = roundf(m_ina_219.voltage() * 1000) / 1000;
      float current = roundf(m_ina_219.current() * 1000) / 1000;
      curr_ina219_voltage=voltage;
      curr_ina219_current=current;
      // debug
      openhd::log::get_default()->debug("Ina219 voltage:{} current:{}",voltage,current);
    }
    if(m_platform.platform_type==PlatformType::RaspberryPi){
      curr_temperature_core=(int8_t)OnboardComputerStatus::rpi::read_temperature_soc_degree();
      // temporary, until we have our own message
      curr_clock_cpu=OnboardComputerStatus::rpi::read_curr_frequency_mhz(OnboardComputerStatus::rpi::VCGENCMD_CLOCK_CPU);
      curr_clock_isp=OnboardComputerStatus::rpi::read_curr_frequency_mhz(OnboardComputerStatus::rpi::VCGENCMD_CLOCK_ISP);
      curr_clock_h264=OnboardComputerStatus::rpi::read_curr_frequency_mhz(OnboardComputerStatus::rpi::VCGENCMD_CLOCK_H264);
      curr_clock_core=OnboardComputerStatus::rpi::read_curr_frequency_mhz(OnboardComputerStatus::rpi::VCGENCMD_CLOCK_CORE);
      curr_clock_v3d=OnboardComputerStatus::rpi::read_curr_frequency_mhz(OnboardComputerStatus::rpi::VCGENCMD_CLOCK_V3D);

    }else{
      const auto cpu_temp=(int8_t)OnboardComputerStatus::readTemperature();
      curr_temperature_core=cpu_temp;
    }
    {
      // lock mutex and write out
      std::lock_guard<std::mutex> lock(m_curr_onboard_computer_status_mutex);
      m_curr_onboard_computer_status.temperature_core[0]=curr_temperature_core;
      // temporary, until we have our own message
      m_curr_onboard_computer_status.storage_type[0]=curr_clock_cpu;
      m_curr_onboard_computer_status.storage_type[1]=curr_clock_isp;
      m_curr_onboard_computer_status.storage_type[2]=curr_clock_h264;
      m_curr_onboard_computer_status.storage_type[3]=curr_clock_core;
      m_curr_onboard_computer_status.storage_usage[0]=curr_clock_v3d;
      m_curr_onboard_computer_status.storage_usage[1]=curr_space_left;
      m_curr_onboard_computer_status.storage_usage[2]=curr_ina219_voltage;
      m_curr_onboard_computer_status.storage_usage[3]=curr_ina219_current;
      m_curr_onboard_computer_status.ram_usage=static_cast<uint32_t>(curr_ram_usage.ram_usage_perc);
      m_curr_onboard_computer_status.ram_total=curr_ram_usage.ram_total_mb;
    }
  }
}
std::vector<MavlinkMessage>
OnboardComputerStatusProvider::get_current_status_as_mavlink_message(const uint8_t sys_id,const uint8_t comp_id) {
  MavlinkMessage msg;
  auto tmp=get_current_status();
  mavlink_msg_onboard_computer_status_encode(sys_id,comp_id,&msg.m,&tmp);
  std::vector<MavlinkMessage> ret;
  ret.push_back(msg);
  return ret;
}

void OnboardComputerStatusProvider::ina219_log_warning_once() {
  if(m_ina_219.has_any_error && !m_ina219_warning_logged){
    openhd::log::get_default()->warn("INA219 failed - no power monitoring");
    m_ina219_warning_logged= true;
  }
}
