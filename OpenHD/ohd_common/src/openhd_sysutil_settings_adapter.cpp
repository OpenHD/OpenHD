// SPDX-License-Identifier: GPL-3.0-only
#include "openhd_sysutil_settings_adapter.h"
#include "openhd_sock.h"
#include "catalog.h"
#include <algorithm>
#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <regex>
#include <thread>

namespace openhd::settings {
namespace {
class State {
 public:
  State(std::string type,SysutilRequest sender,bool background):type(std::move(type)),sender(std::move(sender)) {
    refresh();
    if(background && healthy) worker=std::thread([this]{
      while(running) { std::this_thread::sleep_for(std::chrono::milliseconds(500)); if(running) refresh(); }
    });
  }
  ~State() { running=false; if(worker.joinable()) worker.join(); }
  nlohmann::json read() const { std::lock_guard<std::mutex> lock(mutex); return payload; }
  bool available() const { std::lock_guard<std::mutex> lock(mutex); return healthy; }
  bool apply(const nlohmann::json& request) {
    try { const auto reply=sender(request); const bool ok=reply && reply->value("ok",false); refresh(); return ok; }
    catch(const std::exception&) { return false; }
  }
 private:
  void refresh() {
    std::optional<nlohmann::json> reply;
    try { reply=sender({{"type",type}}); } catch(const std::exception&) {}
    std::lock_guard<std::mutex> lock(mutex);
    healthy=reply && reply->is_object() && (!reply->contains("ok") || (reply->at("ok").is_boolean() && reply->at("ok").get<bool>()));
    if(healthy) payload=*reply; // Keep last readback while the owner is unavailable.
  }
  std::string type;
  SysutilRequest sender;
  mutable std::mutex mutex;
  nlohmann::json payload;
  bool healthy=false;
  std::atomic<bool> running{true};
  std::thread worker;
};

nlohmann::json wire_value(const nlohmann::json& definition,const nlohmann::json& payload) {
  const auto& rpc=definition.at("rpc");
  if(rpc.value("format","")=="display-mode") {
    return std::to_string(payload.at("width").get<int>())+"x"+
      std::to_string(payload.at("height").get<int>())+"@"+std::to_string(payload.at("refresh_hz").get<int>());
  }
  const auto value=payload.at(rpc.at("key").get<std::string>());
  if(rpc.value("boolean",false)) return value.get<bool>() ? 1 : 0;
  return value;
}
}

std::vector<Setting> make_sysutil_settings(SysutilRequest request,bool background_poll) {
  if(!request) request=[](const nlohmann::json& doc) { return openhd::request_sysutil_json(doc,std::chrono::milliseconds(300)); };
  std::map<std::string,std::shared_ptr<State>> states;
  std::vector<Setting> result;
  const auto catalog=nlohmann::json::parse(catalog_json);
  for(const auto& definition:catalog.at("settings")) {
    if(!definition.contains("rpc")) continue;
    const auto& rpc=definition.at("rpc");
    const auto type=rpc.at("read").get<std::string>();
    // Catalog data can bind settings RPCs, never storage actions or arbitrary sockets.
    if(type!="sysutil.settings.request" && type!="sysutil.debug.request" &&
       type!="sysutil.platform.request" && type!="sysutil.display.request") continue;
    auto& state=states[type]; if(!state) state=std::make_shared<State>(type,request,background_poll);
    if(!state->available()) continue;
    if(rpc.contains("availableKey") && !state->read().value(rpc.at("availableKey").get<std::string>(),false)) continue;
    if(rpc.contains("choicesKey") && !state->read().contains(rpc.at("choicesKey").get<std::string>())) continue;
    nlohmann::json initial;
    try { initial=wire_value(definition,state->read()); } catch(const std::exception&) { continue; }
    const auto setter=[definition,state](const nlohmann::json& value) {
      if(!state->available()) return false;
      const auto& rpc=definition.at("rpc");
      if(!rpc.contains("write")) return false;
      const auto write=rpc.at("write").get<std::string>();
      if(write!="sysutil.settings.update" && write!="sysutil.debug.update" && write!="sysutil.display.update") return false;
      nlohmann::json payload={{"type",write}};
      const auto current=state->read();
      for(const auto& key:rpc.value("preserve",nlohmann::json::array())) if(current.contains(key.get<std::string>())) payload[key.get<std::string>()]=current.at(key.get<std::string>());
      if(rpc.value("format","")=="display-mode") {
        std::smatch match; const auto text=value.get<std::string>();
        if(!std::regex_match(text,match,std::regex("^([0-9]+)x([0-9]+)@([0-9]+)$"))) return false;
        try { payload["width"]=std::stoi(match[1]); payload["height"]=std::stoi(match[2]); payload["refresh_hz"]=std::stoi(match[3]); }
        catch(const std::exception&) { return false; }
        payload["enabled"]=true;
      } else payload[rpc.at("key").get<std::string>()]=rpc.value("boolean",false) ? nlohmann::json(value.get<int>()!=0) : value;
      return state->apply(payload);
    };
    Setting setting;
    setting.id=definition.at("id"); setting.read_only=definition.value("readOnly",false);
    const auto metadata=[state,definition] {
      const auto& rpc=definition.at("rpc");
      const bool supported=!rpc.contains("availableKey") || state->read().value(rpc.at("availableKey").get<std::string>(),false);
      nlohmann::json result={{"available",supported},{"writable",state->available()},
        {"reason",state->available() ? "" : "SysUtils is unavailable"}};
      if(rpc.contains("choicesKey")) {
        result["choices"]=nlohmann::json::array();
        const auto payload=state->read();
        const auto allowed=payload.value(rpc.at("choicesKey").get<std::string>(),nlohmann::json::array());
        for(const auto& choice:definition.value("choices",nlohmann::json::array()))
          if(std::find(allowed.begin(),allowed.end(),choice.at("value"))!=allowed.end()) result["choices"].push_back(choice);
      }
      return result;
    };
    setting.metadata=metadata;
    if(definition.at("type")=="int32") {
      setting.setting=IntSetting{initial.get<int>(),[setter](std::string,int n){return setter(n);},
        [state,definition,initial]{try{return wire_value(definition,state->read()).get<int>();}catch(const std::exception&){return initial.get<int>();}}};
    } else {
      setting.setting=StringSetting{initial.get<std::string>(),[setter](std::string,std::string s){return setter(s);},
        [state,definition,initial]{try{return wire_value(definition,state->read()).get<std::string>();}catch(const std::exception&){return initial.get<std::string>();}}};
    }
    result.push_back(std::move(setting));
  }
  return result;
}
}
