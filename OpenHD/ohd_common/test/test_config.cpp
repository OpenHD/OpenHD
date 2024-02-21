//
// Created by consti10 on 20.02.23.
//

#include "openhd_config.h"

int main(int argc, char *argv[]) {
  auto config = openhd::load_config();
  openhd::debug_config(config);
}
