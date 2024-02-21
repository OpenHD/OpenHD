//
// Created by consti10 on 19.03.23.
//

#include "openhd_spdlog.h"
#include "openhd_spdlog_include.h"

int main(int argc, char *argv[]) {
  openhd::log::get_default()->debug("Example debug");
  openhd::log::get_default()->warn("Example warn");
  return 0;
}