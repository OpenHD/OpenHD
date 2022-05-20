//
// Created by consti10 on 15.05.22.
//

#ifndef OPENHD_TESTS_H
#define OPENHD_TESTS_H

#include "openhd-camera.hpp"
#include "openhd-platform.hpp"
#include "openhd-profile.hpp"
#include "openhd-wifi.hpp"

/**
 * Tests for the ohd_common directory.
 */
namespace OHDCommonTests {

// Simple test for to and from string
static void test_video_format_regex() {
  const VideoFormat source{VideoCodecH264, 1280, 720, 30};
  const auto serialized = source.toString();
  const auto from = VideoFormat::fromString(serialized);
  if(!(from == source)){
	throw std::runtime_error("Error VideoFormat from/to\n");
  }
  assert(source == from);
}

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
  if(res2.has_value()){
	std::cout << "Res2 is:[" << res2.value() << "]\n";
	if(res2.value()!="1\n"){
	  throw std::runtime_error("run_command_out return does not match expected\n");
	}
  }
}

}
#endif //OPENHD_TESTS_H
