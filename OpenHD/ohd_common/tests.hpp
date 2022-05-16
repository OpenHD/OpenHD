//
// Created by consti10 on 15.05.22.
//

#ifndef OPENHD_TESTS_H
#define OPENHD_TESTS_H

#include "openhd-camera.hpp"
#include "openhd-platform.hpp"
#include "openhd-profile.hpp"
#include "openhd-wifi.hpp"

namespace OHDCommonTests {

// Simple test for to and from string
static void test_video_format_regex() {
  const VideoFormat source{VideoCodecH264, 1280, 720, 30};
  const auto serialized = source.toString();
  const auto from = VideoFormat::fromString(serialized);
  assert(source == from);
}

}
#endif //OPENHD_TESTS_H
