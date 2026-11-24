// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

namespace openhd::telemetry {

std::vector<std::string> discover_serial_ports(
    const std::filesystem::path& tty_class = "/sys/class/tty",
    const std::filesystem::path& device_directory = "/dev");

nlohmann::json serial_port_metadata();

}  // namespace openhd::telemetry
