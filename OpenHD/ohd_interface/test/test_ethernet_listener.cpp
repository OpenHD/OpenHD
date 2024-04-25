//
// Created by consti10 on 20.05.22.
//

#include "ethernet_manager.h"
#include "openhd_util.h"

int main(int argc, char *argv[]) {
  OHDUtil::terminate_if_not_root();

  auto manager = std::make_unique<EthernetManager>();
  manager->async_initialize(2);

  OHDUtil::keep_alive_until_sigterm();

  manager->stop();
  manager = nullptr;

  return 0;
}