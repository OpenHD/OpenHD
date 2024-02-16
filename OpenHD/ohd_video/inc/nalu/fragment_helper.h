//
// Created by consti10 on 07.01.24.
//

#ifndef OPENHD_FRAGMENT_HELPER_H
#define OPENHD_FRAGMENT_HELPER_H

#include <unistd.h>

static std::vector<std::shared_ptr<std::vector<uint8_t>>> make_fragments(
    const uint8_t* data, int data_len) {
  std::vector<std::shared_ptr<std::vector<uint8_t>>> fragments;
  int bytes_used = 0;
  const uint8_t* p = data;
  static constexpr auto MAX_FRAGMENT_SIZE = 1024;
  while (true) {
    const int remaining = (int)data_len - bytes_used;
    int len = 0;
    if (remaining > MAX_FRAGMENT_SIZE) {
      len = MAX_FRAGMENT_SIZE;
    } else {
      len = remaining;
    }
    std::shared_ptr<std::vector<uint8_t>> fragment =
        std::make_shared<std::vector<uint8_t>>(p, p + len);
    fragments.emplace_back(fragment);
    p = p + len;
    bytes_used += len;
    if (bytes_used == data_len) {
      break;
    }
  }
  return fragments;
}

static std::vector<std::shared_ptr<std::vector<uint8_t>>> drop_fragments(
    std::vector<std::shared_ptr<std::vector<uint8_t>>> fragments) {
  if (fragments.size() > 4) {
    int random = std::rand();
    if (random % 8 == 0) {
      // fragments.resize(fragments.size()/2);
      fragments.resize(0);
    }
  }
  return fragments;
}

#endif  // OPENHD_FRAGMENT_HELPER_H
