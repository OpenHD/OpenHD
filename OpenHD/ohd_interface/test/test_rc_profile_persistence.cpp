#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <type_traits>

#include "include_json.hpp"
#include "wb_link_settings.h"

int main() {
  char temp[] = "/tmp/openhd-rc-profiles-XXXXXX";
  const auto* path = mkdtemp(temp);
  assert(path);
  const std::string directory = std::string(path) + "/";
  const auto file = directory + "wifibroadcast_settings.json";
  const OHDProfile air(true, "test");

  const auto save_profile = [](const auto& settings, const std::string& id,
                               auto value) {
    const auto it = std::find_if(settings.begin(), settings.end(),
        [&](const auto& setting) { return setting.id == id; });
    assert(it != settings.end());
    using SettingType = std::conditional_t<
        std::is_same_v<decltype(value), int>, openhd::IntSetting,
        openhd::StringSetting>;
    return std::get<SettingType>(it->setting).change_callback(id, value);
  };
  {
    openhd::WBLinkSettingsHolder no_radio(air, {}, directory);
    const auto settings = no_radio.get_rc_profile_settings();
    assert(settings.size() == 4);
    assert(!std::filesystem::exists(file));
    assert(save_profile(settings, "MCS_VIA_RC", 10));
    assert(save_profile(settings, "BW_VIA_RC", 12));
    assert(save_profile(settings, "RC_MCS_RANGES",
        "1050:1,1200:2,1350:3,1500:4,1650:1,1800:2,1950:3,2100:4"));
    assert(save_profile(settings, "RC_BW_RANGES", "1500:20,2100:40"));
    assert(!save_profile(settings, "MCS_VIA_RC", 19));
    assert(!save_profile(settings, "RC_BW_RANGES", "1500:80,2100:40"));
    const auto saved = nlohmann::json::parse(OHDFilesystemUtil::read_file(file));
    assert(saved.size() == 4);
    assert(!saved.contains("wb_frequency"));
  }

  WiFiCard card{};
  card.type = WiFiCardType::OPENHD_EMULATED;
  card.supported_frequencies_5G = {openhd::DEFAULT_5GHZ_FREQUENCY};
  {
    openhd::WBLinkSettingsHolder radio(air, {card}, directory);
    assert(radio.created_defaults());
    assert(radio.get_settings().wb_frequency == openhd::DEFAULT_5GHZ_FREQUENCY);
    assert(radio.get_settings().wb_mcs_index_via_rc_channel == 10);
    assert(radio.get_settings().wb_bw_via_rc_channel == 12);
    assert(radio.get_settings().wb_mcs_rc_ranges ==
        "1050:1,1200:2,1350:3,1500:4,1650:1,1800:2,1950:3,2100:4");
    assert(radio.get_settings().wb_bw_rc_ranges == "1500:20,2100:40");
    bool reset_pending_width = false;
    const auto settings = radio.get_rc_profile_settings(
        [&] { reset_pending_width = true; });
    assert(std::none_of(settings.begin(), settings.end(),
        [](const auto& setting) { return setting.seed_allowed; }));
    assert(save_profile(settings, "RC_MCS_RANGES", "1500:4,2100:1"));
    assert(openhd::rc::value_for_pwm(
        radio.get_rc_ranges(&openhd::WBLinkSettings::wb_mcs_rc_ranges),
        openhd::rc::RangeKind::Mcs, 1550) == 1);
    assert(save_profile(settings, "BW_VIA_RC", 0));
    assert(reset_pending_width);
    radio.unsafe_get_settings().wb_frequency = 5825;
    radio.persist();
  }
  {
    openhd::WBLinkSettingsHolder no_radio(air, {}, directory);
    const auto settings = no_radio.get_rc_profile_settings();
    assert(save_profile(settings, "MCS_VIA_RC", 0));
    assert(save_profile(settings, "RC_BW_RANGES", "2100:10"));
  }
  {
    openhd::WBLinkSettingsHolder radio(air, {card}, directory);
    assert(!radio.created_defaults());
    assert(radio.get_settings().wb_frequency == 5825);
    assert(radio.get_settings().wb_mcs_index_via_rc_channel == 0);
    assert(radio.get_settings().wb_bw_rc_ranges == "2100:10");
  }
  std::filesystem::remove_all(directory);
}
