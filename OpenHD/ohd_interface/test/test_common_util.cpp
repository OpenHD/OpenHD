//
// Created by consti10 on 13.05.22.
//
// Should be placed under ohd_common, test the execute command for weird locale issue

#include "openhd-util.hpp"

int main(int argc, char *argv[]) {

  auto res = OHDUtil::run_command("echo", {"1"});
  std::cout << "Res is:" << res << "\n";

  auto res2=OHDUtil::run_command_out("echo 1");
  if(res2.has_value()){
	std::cout << "Res2 is:[" << res2.value() << "]\n";
	if(res2.value()!="1\n"){
	  throw std::runtime_error("run_command_out return does not match expected\n");
	}
  }
}

