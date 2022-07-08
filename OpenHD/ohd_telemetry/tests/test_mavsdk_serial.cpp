//
// Created by consti10 on 09.07.22.
//

#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/mavlink_passthrough/mavlink_passthrough.h>
#include <iostream>
#include <thread>
#include <getopt.h>

static const char optstr[] = "?s:b:";
static const struct option long_options[] = {
    {"serial", required_argument, nullptr, 's'},
    {"baud", required_argument, nullptr, 'b'},
    {nullptr, 0, nullptr, 0},
};


int main(int argc, char *argv[]) {
  int c;
  std::string serial_port_filename="/dev/ttyACM0";
  int baud_rate=115200;
  while ((c = getopt_long(argc, argv, optstr, long_options, NULL)) != -1) {
    const char *tmp_optarg = optarg;
    switch (c) {
      case 's':
        serial_port_filename=std::string(tmp_optarg);
        break;
      case 'b':
        baud_rate=std::atoi(tmp_optarg);
        break;
      case '?':
      default:
        std::cout << "Usage: \n" <<
            "--serial -s [serial port string filename, default: /dev/ttyACMO]\n"<<
            "--baud -b [baud rate, default: 115200]\n";
        break;
    }
  }
  //ohd_log(STATUS_LEVEL_EMERGENCY, "Test log message\n");

  auto mavsdk=std::make_shared<mavsdk::Mavsdk>();
  std::shared_ptr<mavsdk::System> system;
  std::shared_ptr<mavsdk::MavlinkPassthrough> passthrough;
  mavsdk->subscribe_on_new_system([&mavsdk,&system,&passthrough]() {
    system = mavsdk->systems().back();
    std::cout << "System found"<<(int)system->get_system_id()<<"\n";
    /*passthrough=std::make_shared<mavsdk::MavlinkPassthrough>(system);
    passthrough->intercept_incoming_messages_async([](mavlink_message_t& msg){
      std::cout<<"Got message\n";
    });*/
  });
  mavsdk::ConnectionResult connection_result=mavsdk::ConnectionResult::ConnectionError;
  while (connection_result!= mavsdk::ConnectionResult::Success){
    std::cout<<"try adding serial";
    connection_result = mavsdk->add_serial_connection(serial_port_filename,baud_rate);
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  while (true){
    std::cout<<"Running\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

}