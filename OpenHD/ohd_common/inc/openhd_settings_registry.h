// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "openhd_settings_imp.h"
#include "catalog.h"
#include <map>

namespace openhd::settings {
inline nlohmann::json describe(const Setting& setting) {
  static const auto definitions=[] {
    std::map<std::string,nlohmann::json> result;
    const auto catalog=nlohmann::json::parse(catalog_json);
    for (const auto& entry : catalog.at("settings")) result.emplace(entry.at("id"),entry);
    return result;
  }();
  nlohmann::json result={{"id",setting.id},{"label",setting.id},{"group","General"}};
  const auto known=definitions.find(setting.id);
  if (known!=definitions.end()) result=known->second;
  const bool integer=std::holds_alternative<IntSetting>(setting.setting);
  // The registered implementation, not a stale catalog entry, determines type.
  result["type"]=integer ? "int32" : "string";
  result["available"]=true;
  if (setting.metadata) result.update(setting.metadata());
  result["readOnly"]=setting.read_only || result.value("readOnly",false);
  if (integer && !std::get<IntSetting>(setting.setting).change_callback) result["writable"]=false;
  if (!integer && !std::get<StringSetting>(setting.setting).change_callback) result["writable"]=false;
  return result;
}
inline bool valid(const Setting& setting,const nlohmann::json& value) {
  const auto meta=describe(setting);
  if (!meta.value("available",true) || meta.value("readOnly",false) || !meta.value("writable",true)) return false;
  if (std::holds_alternative<IntSetting>(setting.setting)) {
    if (!value.is_number_integer()) return false;
    if (value.is_number_unsigned() && value.get<uint64_t>()>uint64_t(INT32_MAX)) return false;
    const auto n=value.get<int64_t>();
    if (n<INT32_MIN || n>INT32_MAX) return false;
  } else if (!value.is_string() || value.get<std::string>().size()>128 || value.get<std::string>().find('\0')!=std::string::npos) return false;
  // Imported client descriptions are advisory until audited. Backend callbacks
  // remain authoritative; new definitions opt into shared constraint validation.
  if (!meta.value("enforce",false)) return true;
  if (meta.contains("minimum") && value<meta["minimum"]) return false;
  if (meta.contains("maximum") && value>meta["maximum"]) return false;
  if (meta.contains("choices")) {
    for (const auto& choice:meta["choices"]) if (choice.at("value")==value) return true;
    return false;
  }
  return true;
}
inline nlohmann::json snapshot(const std::vector<Setting>& settings,uint8_t sys,uint8_t comp) {
  nlohmann::json entries=nlohmann::json::array();
  for (const auto& setting:settings) {
    if (setting.id=="DUMMY") continue;
    auto meta=describe(setting);
    meta.erase("rpc");
    if (!meta.value("available",true)) continue;
    entries.push_back(std::move(meta));
  }
  return {{"version",1},{"systemId",sys},{"componentId",comp},{"settings",entries}};
}
inline nlohmann::json standard_parameters(const nlohmann::json& registry) {
  auto parameters=nlohmann::json::array();
  for (const auto& entry:registry.at("settings")) {
    // Current upstream schema only represents numeric parameters.
    if (entry.at("type")!="int32") continue;
    nlohmann::json p={{"name",entry.at("id")},{"type","Int32"},
      {"shortDesc",entry.value("label",entry.at("id").get<std::string>())},
      {"longDesc",entry.value("description","")},{"group",entry.value("group","General")},
      {"readOnly",entry.value("readOnly",false)},{"rebootRequired",entry.value("requiresReboot",false)}};
    if (entry.contains("minimum")) p["min"]=entry["minimum"];
    if (entry.contains("maximum")) p["max"]=entry["maximum"];
    if (entry.contains("unit")) p["units"]=entry["unit"];
    if (entry.contains("choices")) {
      p["values"]=nlohmann::json::array();
      for (const auto& c:entry["choices"]) p["values"].push_back({{"value",c.at("value")},{"description",c.at("label")}});
    }
    parameters.push_back(std::move(p));
  }
  return {{"version",3},{"parameters",parameters}};
}
}
