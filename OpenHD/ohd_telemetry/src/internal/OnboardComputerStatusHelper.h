//
// Created by consti10 on 27.07.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_ONBOARDCOMPUTERSTATUSHELPER_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_ONBOARDCOMPUTERSTATUSHELPER_H_

#include "openhd-util.hpp"
#include <thread>
#include <memory>
#include <cstdlib>
#include <cmath>

namespace openhd{

// really stupid, but for now the best solution I came up with
// loosely based on https://stackoverflow.com/questions/9229333/how-to-get-overall-cpu-usage-e-g-57-on-linux
// NOTE: top -v returns procps-ng on both pi4 and my ubuntu laptop
// Also note, we want the CPU usage from all processes - not only -p 1
static std::optional<int> read_cpuload_once_blocking(){
  //auto res=OHDUtil::run_command_out(R"lit(top -b -n1  | fgrep "Cpu(s)" | tail -1 | awk -F'id,' -v prefix="$prefix" '{ split($1, vs, ","); v=vs[length(vs)]; sub("%", "", v); printf "%s%.1f\n", prefix, 100 - v }')lit");
  auto res=OHDUtil::run_command_out(R"lit(top -bn2 | grep "Cpu(s)" |  sed "s/.*, *\([0-9.]*\)%* id.*/\1/" | awk '{print 100 - $1"%"}')lit");
  if(res.has_value()){
	std::cout<<"read_cpuload_once_blocking:"<<res.value()<<"\n";
	const auto value=std::atof(res.value().c_str());
	// we don't care about the decimals
	const int value_as_int=static_cast<int>(lround(value));
	std::cout<<"read_cpuload_once_blocking: "<<value_as_int<<" %";
	return value_as_int;
  }
  return std::nullopt;
}


// Note: we need to run this in its own thread, such that "top" can do its magic over a longer duration
class CPUUsageCalculator{
 public:
  CPUUsageCalculator(){
	calculate_thread=std::make_unique<std::thread>(&CPUUsageCalculator::calculateInfinite, this);
  }
  int get_last_cpu_usage()const{
	return last_cpu_usage_percent;
  }
 private:
  void calculateInfinite(){
	while (true){
	  const auto before=std::chrono::steady_clock::now();
	  const auto value=read_cpuload_once_blocking();
	  const auto read_time=std::chrono::steady_clock::now()-before;
	  if(value.has_value()){
		last_cpu_usage_percent=value.value();
	  }
	  //std::cout<<"Took:"<<std::chrono::duration_cast<std::chrono::milliseconds>(read_time).count()<<"\n";
	  // top should block for around 3 seconds, but in case it doesn't make sure we don't neccessarily waste cpu here
	  if(read_time<=std::chrono::seconds(3)){
		const auto duration=std::chrono::seconds(3)-read_time;
		std::this_thread::sleep_for(duration);
	  }
	}
  }
  int last_cpu_usage_percent=0;
  std::unique_ptr<std::thread> calculate_thread;
};

}

#endif //OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_ONBOARDCOMPUTERSTATUSHELPER_H_
