// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "openhd_settings_imp.h"
#include <optional>
namespace openhd::settings {
using SysutilRequest=std::function<std::optional<nlohmann::json>(const nlohmann::json&)>;
std::vector<Setting> make_sysutil_settings(SysutilRequest request=nullptr,bool background_poll=true);
}
