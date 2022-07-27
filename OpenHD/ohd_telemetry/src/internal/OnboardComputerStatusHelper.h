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
#include <regex>

namespace openhd{



// really stupid, but for now the best solution I came up with
// loosely based on https://stackoverflow.com/questions/9229333/how-to-get-overall-cpu-usage-e-g-57-on-linux
// NOTE: top -v returns procps-ng on both pi4 and my ubuntu laptop
// Also note, we want the CPU usage from all processes - not only -p 1
// 28.July 2022: This seems to work on both rpi4 and my ubuntu pc.
// Also, I am pretty sure we can use -bn1 - top should report from "the last refresh."
static std::optional<int> read_cpuload_once_blocking(){
  auto res_opt=OHDUtil::run_command_out(R"lit(top -bn1 | grep "Cpu(s)")lit");
  // The result from that should look like this: %Cpu(s): 31,0 us,  2,0 sy,  0,0 ni, 67,0 id,  0,0 wa,  0,0 hi,  0,0 si,  0,0 st
  // Where "67.0 id" is what we are after - "time spent in the kernel idle handler"
  // from that, we can deduce the usage
  if(!res_opt.has_value()){
	return std::nullopt;
  }
  const std::string res=res_opt.value();
  //std::cout<<"read_cpuload_once_blocking res:{"<<res<<"}\n";
  std::smatch result;
  const std::regex r1{"ni,(.*) id"};
  auto res1 = std::regex_search(res, result, r1);
  if(!res1 || result.size()<1){
	return std::nullopt;
  }
  const std::string intermediate1=result[0];
  //std::cout<<"Intermediate:{"<<intermediate1<<"}\n";
  if(intermediate1.length()<3){
	return std::nullopt;
  }
  std::regex begin("ni,");
  const auto intermediate2=std::regex_replace(intermediate1, begin, "");
  //std::cout<<"Intermediate2:{"<<intermediate2<<"}\n";
  const auto cpu_idle_perc=std::atof(intermediate2.c_str());
  //std::cout<<"cpu_idle_perc:{"<<cpu_idle_perc<<"}\n";
  const auto cpu_idle_perc_int=static_cast<int>(lround(cpu_idle_perc));
  return 100-cpu_idle_perc_int;
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
	  // top can block up to X seconds, but in case it doesn't make sure we don't neccessarily waste cpu here
	  if(read_time<=std::chrono::seconds(1)){
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
