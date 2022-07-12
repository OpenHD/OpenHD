//
// Created by consti10 on 09.07.22.
//

#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/mavlink_passthrough/mavlink_passthrough.h>
#include <iostream>
#include <thread>

#include "serial_endpoint_test_helper.h"
#include "mav_include.h"

#include "serial_endpoint_test_helper.h"

int main(int argc, char *argv[]) {
  const auto serial_options=serial_endpoint_test_helper::parse_args(argc,argv);
  //ohd_log(STATUS_LEVEL_EMERGENCY, "Test log message\n");

  auto mavsdk=std::make_shared<mavsdk::Mavsdk>();
  std::shared_ptr<mavsdk::System> system;
  std::shared_ptr<mavsdk::MavlinkPassthrough> passthrough;
  mavsdk->subscribe_on_new_system([&mavsdk,&system,&passthrough]() {
    system = mavsdk->systems().back();
    std::cout << "System found"<<(int)system->get_system_id()<<"\n";
    passthrough=std::make_shared<mavsdk::MavlinkPassthrough>(system);
    passthrough->intercept_incoming_messages_async([](mavlink_message_t& msg){
      std::cout<<"Got message\n";
      return true;
    });
  });
  mavsdk::ConnectionResult connection_result=mavsdk::ConnectionResult::ConnectionError;
  while (connection_result!= mavsdk::ConnectionResult::Success){
    std::cout<<"try adding serial";
    connection_result = mavsdk->add_serial_connection(serial_options.filename,serial_options.baud_rate);
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  while (true){
    std::cout<<"Running\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

}