#pragma once
#include <algorithm>
#include <cmath>
#include <chrono>
#include <fstream>
#include <optional>
#include <vector>
#include "include_json.hpp"

namespace openhd {
struct EspRfChannel { int frequency_mhz; uint16_t busy_centipercent; };
struct EspAnalysis {
  std::vector<int> priority_mhz;
  std::vector<EspRfChannel> channels;
};
// Atomic JSON produced by esp-sdr's background advisor; never controls a radio.
inline std::optional<EspAnalysis> read_esp_analysis(
    const std::string& path, int64_t now_ms) {
  try {
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input || input.tellg() < 0 || input.tellg() > 1024*1024) return std::nullopt;
    input.seekg(0);
    const auto j=nlohmann::json::parse(input);
    if (j.at("schema")!="openhd.rf_advice" || j.at("version")!=1 ||
        !j.at("connected").get<bool>() || !j.at("scanning").get<bool>()) return std::nullopt;
    const auto age=now_ms-j.at("generated_unix_ms").get<int64_t>();
    const auto& d=j.at("discovery");
    if (d.at("schema")!="openhd.rf_discovery" || d.at("version")!=1) return std::nullopt;
    const auto expiry=std::min<int64_t>(3000,std::min(j.at("expires_after_ms").get<int64_t>(),d.at("expires_after_ms").get<int64_t>()));
    if (age<0 || expiry<=0 || age>=expiry) return std::nullopt;
    EspAnalysis result;
    for (const auto& c:j.at("channels")) {
      if (!c.contains("age_seconds") || !c.contains("sampled_busy_percent") || c.at("sampled_busy_percent").is_null()) continue;
      const double channel_age=c.at("age_seconds").get<double>();
      const double busy=c.at("sampled_busy_percent").get<double>();
      if (!std::isfinite(channel_age) || channel_age<0 || channel_age+age/1000.0>3 || !std::isfinite(busy) || busy<0 || busy>100) continue;
      bool invalid=false;
      for (const auto& reason:c.at("reason_codes"))
        if (reason=="STALE_OR_GAIN_UNSTABLE" || reason=="ADC_CLIPPING") invalid=true;
      const auto hz=c.at("frequency_hz").get<int64_t>();
      if (invalid || hz%1000000 || hz<2000000000LL || hz>6000000000LL) continue;
      const int mhz=static_cast<int>(hz/1000000);
      if (std::any_of(result.channels.begin(),result.channels.end(),[&](const auto& v){return v.frequency_mhz==mhz;})) continue;
      result.channels.push_back({mhz,static_cast<uint16_t>(std::lround(busy*100))});
    }
    for (const auto& v:d.at("priority_frequencies_hz")) {
      const auto hz=v.get<int64_t>();
      if(hz%1000000) continue;
      const int mhz=static_cast<int>(hz/1000000);
      if(std::find(result.priority_mhz.begin(),result.priority_mhz.end(),mhz)!=result.priority_mhz.end()) continue;
      if(std::any_of(result.channels.begin(),result.channels.end(),[&](const auto& c){return c.frequency_mhz==mhz;})) result.priority_mhz.push_back(mhz);
    }
    if(result.channels.empty()) return std::nullopt;
    return result;
  } catch (const std::exception&) { return std::nullopt; }
}
template<class Channel> void prioritize_esp_channels(std::vector<Channel>& channels,const EspAnalysis& advice) {
  const auto rank=[&](int mhz){return std::find(advice.priority_mhz.begin(),advice.priority_mhz.end(),mhz)-advice.priority_mhz.begin();};
  std::stable_sort(channels.begin(),channels.end(),[&](const auto& a,const auto& b){return rank(a.frequency)<rank(b.frequency);});
}
inline std::optional<EspAnalysis> latest_esp_analysis() {
  const auto now=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  return read_esp_analysis("/run/openhd-esp/rf-advice.json",now);
}
}
