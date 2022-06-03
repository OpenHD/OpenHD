//
// Created by consti10 on 27.04.22.
// For testing, run the ground code such that it can communicate with QOpenHD, but there is no air unit.
//
#include <iostream>

#include "../src/GroundTelemetry.h"
#include "../src/OHDTelemetry.hpp"
#include "openhd-profile.hpp"
#include "openhd-platform.hpp"
#include <thread>
#include <memory>

static constexpr auto TAG = "TEST_GROUND_LOCALLY";

int main() {
  std::cout << TAG << "start\n";
  std::unique_ptr<OHDTelemetry> ground;
  //std::this_thread::sleep_for(std::chrono::seconds(10));
  {
	OHDProfile profile{false, "XX"};
	OHDPlatform platform{};
	ground = std::make_unique<OHDTelemetry>(platform, profile);
  }
  while (true) {
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::cout<<ground->createDebug()<<"\n";
  }
}