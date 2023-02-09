//
// Created by consti10 on 15.05.22.
//

#ifndef OPENHD_TESTS_H
#define OPENHD_TESTS_H

//#include "openhd-camera.hpp"
#include "openhd-platform.hpp"
#include "openhd-profile.hpp"
#include "wifi_card.hpp"

/**
 * Tests for the ohd_common directory.
 */
namespace OHDCommonTests {

static void test_execute_commands(){
  // We do echo 1, but the method should return "0" which stands for
  // command succesfully executed
  auto res = OHDUtil::run_command("echo", {"1"});
  std::cout << "Res is:" << res << "\n";
  if(res!=0){
	throw std::runtime_error("run_command return does not match expected\n");
  }
  // Here we get the actual output in the shell, which should be 1
  auto res2=OHDUtil::run_command_out("echo 1");
  std::cout << "Res2 is:[" << res2.value() << "]\n";
  if(res2!="1\n"){
	throw std::runtime_error("run_command_out return does not match expected\n");
  }
  auto res3=OHDUtil::run_command_out("rambazambathiscommanddoesnotexist");
  if(res3!=std::nullopt){
	std::cerr<<"res3 is:["<<res3.value()<<"]\n";
	//throw std::runtime_error("run_command_out an unknown command should return command not found\n");
  }
}

}
#endif //OPENHD_TESTS_H
