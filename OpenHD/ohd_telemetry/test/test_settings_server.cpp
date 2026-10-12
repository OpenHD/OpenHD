#include "mavsdk_temporary/SettingsMavlinkServer.h"
#include <chrono>
#include <iostream>
#include <thread>
const char* getVideoPath() { return "/tmp/"; }
int main() {
  int value=0;
  openhd::settings::SettingsMavlinkServer server(101,{{100,{
    {"NEW_TEST",openhd::IntSetting{0,[&](std::string,int n){value=n;return true;},[&]{return value;}},
      []{return nlohmann::json{{"enforce",true},{"minimum",0},{"maximum",2}};}},
    openhd::create_read_only_int("READ_ONLY",7),
    {"TEXT_TEST",openhd::StringSetting{"initial",[](std::string,std::string){return true;}}}
  }}},15760);
  std::cout << "READY" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(20));
}
