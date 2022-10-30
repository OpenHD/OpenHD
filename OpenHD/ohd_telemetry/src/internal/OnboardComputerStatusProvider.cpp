//
// Created by consti10 on 30.10.22.
//

#include "OnboardComputerStatusProvider.h"


OnboardComputerStatusProvider::OnboardComputerStatusProvider(OHDPlatform platform): m_platform(platform) {
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
    if(m_platform.platform_type==PlatformType::RaspberryPi){
      curr_temperature_core=(int8_t)OnboardComputerStatus::rpi::read_temperature_soc_degree();
      // temporary, until we have our own message
      curr_clock_cpu=OnboardComputerStatus::rpi::read_curr_frequency_mhz(OnboardComputerStatus::rpi::VCGENCMD_CLOCK_CPU);
      curr_clock_isp=OnboardComputerStatus::rpi::read_curr_frequency_mhz(OnboardComputerStatus::rpi::VCGENCMD_CLOCK_ISP);
      curr_clock_h264=OnboardComputerStatus::rpi::read_curr_frequency_mhz(OnboardComputerStatus::rpi::VCGENCMD_CLOCK_H264);
      curr_clock_core=OnboardComputerStatus::rpi::read_curr_frequency_mhz(OnboardComputerStatus::rpi::VCGENCMD_CLOCK_CORE);
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
