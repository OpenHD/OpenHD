//
// Created by consti10 on 13.06.22.
//

#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/param_server/param_server.h>
#include <iostream>
#include <thread>

int main() {
  //ohd_log(STATUS_LEVEL_EMERGENCY, "Test log message\n");

  auto mavsdkServer=std::make_shared<mavsdk::Mavsdk>();

  mavsdkServer->subscribe_on_new_system([]() {
	std::cout << "System found\n";
  });

  mavsdk::Mavsdk::Configuration configuration(
	  100,190, true);
  mavsdkServer->set_configuration(configuration);

  auto result = mavsdkServer->add_any_connection("udp://127.0.0.1:14550");
  if (result == mavsdk::ConnectionResult::Success) {
	std::cout << "Connected server side!" << std::endl;
  }

  auto server_component =
	  mavsdkServer->server_component_by_type(mavsdk::Mavsdk::ServerComponentType::CompanionComputer);

  // Create server plugins
  auto paramServer = mavsdk::ParamServer{server_component};

  paramServer.provide_param_int("CAL_ACC0_ID", 1);
  paramServer.provide_param_int("CAL_GYRO0_ID", 1);
  paramServer.provide_param_int("CAL_MAG0_ID", 1);
  paramServer.provide_param_int("SYS_HITL", 0);
  paramServer.provide_param_int("MIS_TAKEOFF_ALT", 0);
  paramServer.provide_param_int("OHD_UART_BAUD", 22);
  paramServer.provide_param_custom("OHD_UART_NAME","/dev/ttyUSB0");

  //auto result2=paramServer.retrieve_param_int("CAL_ACC0_ID");

  while (true){
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::cout<<paramServer.retrieve_param_int("OHD_UART_BAUD").second<<"\n";
  }

}