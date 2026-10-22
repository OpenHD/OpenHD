#include "x21_camera_detection.h"

#include <cstdlib>
#include <iostream>

int main() {
  const auto require = [](bool value) { if (!value) std::exit(2); };
  char temporary[] = "/tmp/openhd-x21-camera-XXXXXX";
  const auto* directory = mkdtemp(temporary);
  if (!directory) return 1;
  const std::filesystem::path root(directory);
  const auto add_device = [&](const char* node, const char* name) {
    std::filesystem::create_directories(root / node);
    std::ofstream(root / node / "name") << name << '\n';
  };
  require(!openhd::x21::detect_camera_type(root / "missing"));
  add_device("v4l-subdev0", "rockchip-mipi-csi2");
  add_device("video0", "arducam-pivariety");
  require(!openhd::x21::detect_camera_type(root));
  add_device("v4l-subdev2", "arducam-pivariety 5-000c");
  require(openhd::x21::detect_camera_type(root) ==
         X_CAM_TYPE_X21_ARDUCAM_PIVARIETY);
  require(openhd::x21::is_native_csi_type(X_CAM_TYPE_ROCKCHIP_RV1126_CSI));
  require(openhd::x21::is_native_csi_type(X_CAM_TYPE_X21_ARDUCAM_PIVARIETY));
  require(!openhd::x21::is_native_csi_type(X_CAM_TYPE_EXTERNAL_IP));
  require(!openhd::x21::is_native_csi_type(X_CAM_TYPE_USB_GENERIC));
  std::filesystem::remove_all(root);
  std::cout << "PASS: Arducam subdevice detection without guessing an IMX model\n";
}
