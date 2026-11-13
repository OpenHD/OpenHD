#include "v4l2_camera_selection.h"
#include <cstdlib>
#include <iostream>

int main() {
  using namespace openhd::v4l2;
  const auto require = [](bool passed) { if (!passed) std::exit(2); };
  require(is_usb_or_loopback("usb-0000:01:00.0-1.2.4", "uvcvideo"));
  require(is_usb_or_loopback("platform:v4l2loopback-000", "v4l2 loopback"));
  require(!is_usb_or_loopback("platform:fe801000.csi", "unicam"));
  require(!is_usb_or_loopback("platform:bcm2835-isp", "bcm2835-isp"));
  char temporary[] = "/tmp/openhd-v4l2-selection-XXXXXX";
  const auto* directory = mkdtemp(temporary);
  require(directory != nullptr);
  const std::filesystem::path root(directory);
  const auto add = [&](const char* node, const char* name) {
    std::filesystem::create_directories(root / node);
    std::ofstream(root / node / "name") << name << '\n';
  };
  add("video0", "T2");
  add("video1", "T2");
  require(!find_veye_capture_device(root));
  add("video2", "unicam-image");
  add("video3", "unicam-embedded");
  require(find_veye_capture_device(root) == "/dev/video2");
  std::filesystem::remove_all(root / "video2");
  add("video0", "unicam-image");
  require(find_veye_capture_device(root) == "/dev/video0");
  add("video4", "unicam-image");
  require(!find_veye_capture_device(root));
  require(!find_rockchip_isp_mainpath(root));
  add("video5", "rkisp_selfpath");
  add("video11", "rkisp_mainpath");
  require(find_rockchip_isp_mainpath(root) == "/dev/video11");
  add("video12", "rkisp_mainpath");
  require(!find_rockchip_isp_mainpath(root));
  std::filesystem::remove_all(root);
  std::cout << "V4L2 CSI/USB isolation and VEYE node selection passed\n";
}
