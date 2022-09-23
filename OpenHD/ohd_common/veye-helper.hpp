//
// Created by consti10 on 13.08.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_VEYE_HELPER_H_
#define OPENHD_OPENHD_OHD_COMMON_VEYE_HELPER_H_

#include "openhd-util.hpp"
#include <sstream>

// NOTE: This is a bad software design, but r.n the veye is kinda special and I hacked it in "working" in the quickest way possible.
// Please do not use this as an example on how to do cameras, look at gstreamerstream ;)

namespace openhd::veye{

// TODO: does this also kill OpenHD, since it is started from OpenHD ?!
// Answer: I am pretty sure it doesn't.
static void kill_all_running_veye_instances(){
  std::stringstream command;
  command<<"killall "<<"\"/usr/local/share/veye-raspberrypi/veye_raspivid\"";
  auto res=OHDUtil::run_command_out(command.str().c_str());
  if(res.has_value()){
	std::stringstream ss;
	ss<<"Killall veye returned:{"<<res.value()<<"}\n";
	std::cout<<ss.str();
  }
}



}
#endif //OPENHD_OPENHD_OHD_COMMON_VEYE_HELPER_H_
