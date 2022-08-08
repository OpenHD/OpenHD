// For the onboard computer status we read a lot of stuff,
// This is for testing these functionalities

#include "../src/internal/OnboardComputerStatus.hpp"
#include "openhd-platform.hpp"
#include "openhd-platform-discover.hpp"

int main() {

  const auto platform=DPlatform::discover();
  const auto cpu_load=openhd::read_cpuload_once_blocking();
  if(cpu_load.has_value()){
	std::cout<<"cpu load:"<<cpu_load.value()<<"\n";
  }
  std::cout<<"temp:"<<OnboardComputerStatus::readTemperature()<<"\n";
  if(platform->platform_type==PlatformType::RaspberryPi || true){
    std::cout<<"Rpi:\n";
    std::cout<<""<<(int)OnboardComputerStatus::rpi::read_temperature_soc_degree()<<"\n";
  }
  return 0;
}