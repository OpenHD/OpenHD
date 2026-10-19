/******************************************************************************
 * OpenHD
 *
 * Licensed under the GNU General Public License (GPL) Version 3.
 *
 * This software is provided "as-is," without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose, and non-infringement. For details, see the
 * full license in the LICENSE file provided with this source code.
 *
 * Non-Military Use Only:
 * This software and its associated components are explicitly intended for
 * civilian and non-military purposes. Use in any military or defense
 * applications is strictly prohibited unless explicitly and individually
 * licensed otherwise by the OpenHD Team.
 *
 * Contributors:
 * A full list of contributors can be found at the OpenHD GitHub repository:
 * https://github.com/OpenHD
 *
 * © OpenHD, All Rights Reserved.
 ******************************************************************************/

#include "XMavlinkParamProvider.h"

#include <openhd_spdlog.h>
#include "openhd_settings_registry.h"
#include "openhd_sysutil_settings_adapter.h"
#include <cstring>
#include <fstream>
#include <filesystem>
#include <cstdlib>

XMavlinkParamProvider::XMavlinkParamProvider(
    uint8_t sys_id, uint8_t comp_id,
    std::optional<std::chrono::milliseconds> opt_heartbeat_interval)
    : MavlinkComponent(sys_id, comp_id),
      m_opt_heartbeat_interval(opt_heartbeat_interval) {
  _sender = std::make_shared<mavsdk::SenderWrapper>(*this);
  _mavlink_message_handler = std::make_shared<mavsdk::MavlinkMessageHandler>();
  _mavlink_parameter_receiver =
      std::make_shared<mavsdk::MavlinkParameterReceiver>(
          *_sender, *_mavlink_message_handler);
}

void XMavlinkParamProvider::add_param(const openhd::Setting& setting) {
  m_registered_settings.push_back(setting);
  if (std::holds_alternative<openhd::IntSetting>(setting.setting)) {
    const auto intSetting = std::get<openhd::IntSetting>(setting.setting);
    const auto result = _mavlink_parameter_receiver->provide_server_param<int>(
        setting.id, intSetting.value,
        [setting, intSetting](std::string id, int value) {
          return openhd::settings::valid(setting,value) && intSetting.change_callback && intSetting.change_callback(id,value);
        },
        // A link-controlled encoder can differ from the cached manual target.
        setting.id == "BITRATE_MBITS");
    assert(result == mavsdk::MavlinkParameterReceiver::Result::Success);
    if (intSetting.get_callback != nullptr) {
      m_int_settings_with_update_functionality.push_back(setting);
    }
  } else if (std::holds_alternative<openhd::StringSetting>(setting.setting)) {
    const auto stringSetting = std::get<openhd::StringSetting>(setting.setting);
    const auto result =
        _mavlink_parameter_receiver->provide_server_param<std::string>(
            setting.id, stringSetting.value,
            [setting, stringSetting](std::string id, std::string value) {
              return openhd::settings::valid(setting,value) && stringSetting.change_callback && stringSetting.change_callback(id,value);
            });
    assert(result == mavsdk::MavlinkParameterReceiver::Result::Success);
    if (stringSetting.get_callback != nullptr) {
      m_string_settings_with_update_functionality.push_back(setting);
    }
  } else {
    assert(false);
  }
}

void XMavlinkParamProvider::add_params(
    const std::vector<openhd::Setting>& settings) {
  for (const auto& setting : settings) {
    add_param(setting);
  }
}

void XMavlinkParamProvider::set_ready() {
  if(m_ready) return;
  if(m_comp_id==MAV_COMP_ID_ONBOARD_COMPUTER) add_params(openhd::settings::make_sysutil_settings());
  _mavlink_parameter_receiver->set_request_validator([this](const std::string& id,const mavsdk::ParamValue& value){
    const auto it=std::find_if(m_registered_settings.begin(),m_registered_settings.end(),[&](const auto& s){return s.id==id;});
    if (it==m_registered_settings.end()) return false;
    if (std::holds_alternative<openhd::IntSetting>(it->setting))
      return value.is<int32_t>() && openhd::settings::valid(*it,value.get<int32_t>());
    return value.is<std::string>() && openhd::settings::valid(*it,value.get<std::string>());
  });
  refresh_metadata();
  m_ready = true;
  apply_initial_configuration();
  _mavlink_parameter_receiver->ready_for_communication();
}

void XMavlinkParamProvider::apply_initial_configuration() {
  const char* override_path=std::getenv("OPENHD_INITIAL_SETTINGS_FILE");
  const std::vector<std::string> paths=override_path ? std::vector<std::string>{override_path} :
    std::vector<std::string>{"/Config/settings.json","/Config/openhd/settings.json","/boot/openhd/settings.json"};
  for (const auto& path:paths) {
    try {
      if (!std::filesystem::exists(path) || std::filesystem::file_size(path)>1024*1024) continue;
      std::ifstream file(path); nlohmann::json doc; file>>doc;
      if (!doc.contains("parameter_defaults")) return;
      const auto role=doc.value("role","");
      if ((m_sys_id==101 && role!="air") || (m_sys_id==100 && role!="ground")) return;
      const auto& plan=doc.at("parameter_defaults");
      const auto key=std::to_string(m_comp_id);
      if (plan.at("version")!=1 || !plan.at("components").contains(key)) return;
      const auto& values=plan.at("components").at(key);
      if (!values.is_object()) throw std::runtime_error("Invalid initial parameter defaults");
      auto report=nlohmann::json::object();
      for (auto it=values.begin();it!=values.end();++it) {
        const auto found=std::find_if(m_registered_settings.begin(),m_registered_settings.end(),[&](const auto& s){return s.id==it.key();});
        if (found==m_registered_settings.end() || !found->seed_allowed) { report[it.key()]="unsupported-or-existing-settings"; continue; }
        if (!openhd::settings::valid(*found,it.value())) { report[it.key()]="invalid"; continue; }
        bool accepted=false;
        if (std::holds_alternative<openhd::IntSetting>(found->setting)) {
          const auto& setting=std::get<openhd::IntSetting>(found->setting); const auto value=it.value().get<int>();
          accepted=setting.change_callback && setting.change_callback(found->id,value);
          if (accepted) _mavlink_parameter_receiver->update_existing_server_param_int(found->id,setting.get_callback ? setting.get_callback() : value);
        } else {
          const auto& setting=std::get<openhd::StringSetting>(found->setting); const auto value=it.value().get<std::string>();
          accepted=setting.change_callback && setting.change_callback(found->id,value);
          if (accepted) _mavlink_parameter_receiver->update_existing_server_param_custom(found->id,setting.get_callback ? setting.get_callback() : value);
        }
        report[it.key()]=accepted ? "seeded" : "rejected";
      }
      m_metadata_ftp.files["/openhd/initial-results.json"]=nlohmann::json{{"version",1},{"results",report}}.dump();
    } catch (const std::exception& e) { openhd::log::get_default()->warn("Invalid initial settings: {}",e.what()); }
    return;
  }
}

void XMavlinkParamProvider::refresh_metadata() {
  const auto registry=openhd::settings::snapshot(m_registered_settings,m_sys_id,m_comp_id);
  auto& files=m_metadata_ftp.files;
  files["/openhd/settings.json"]=registry.dump();
  files["/component_metadata/parameters.json"]=openhd::settings::standard_parameters(registry).dump();
  nlohmann::json general={{"version",1},{"vendorName","OpenHD"},{"modelName","OpenHD settings"},
    {"metadataTypes",nlohmann::json::array({{{"type",1},{"uri","mftp:///component_metadata/parameters.json"}}})}};
  files["/component_metadata/general.json"]=general.dump();
}

bool XMavlinkParamProvider::process_metadata(const mavlink_message_t& message) {
  if (!m_ready) return false;
  if (message.msgid==MAVLINK_MSG_ID_FILE_TRANSFER_PROTOCOL) {
    mavlink_file_transfer_protocol_t ftp{};
    mavlink_msg_file_transfer_protocol_decode(&message,&ftp);
    if (ftp.target_system!=m_sys_id || ftp.target_component!=m_comp_id) return false;
    openhd::settings::FtpPayload request{};
    std::copy_n(ftp.payload,251,request.begin());
    // Only resolve capabilities when opening a snapshot, not for every chunk.
    if (request[3]==4 || request[3]==14) refresh_metadata();
    if (const auto response=m_metadata_ftp.handle(message.sysid,message.compid,request)) {
      mavlink_message_t out{};
      mavlink_msg_file_transfer_protocol_pack(m_sys_id,m_comp_id,&out,0,message.sysid,message.compid,response->data());
      _sender->send_message(out);
    }
    return true;
  }
  if (message.msgid==MAVLINK_MSG_ID_COMMAND_LONG) {
    mavlink_command_long_t cmd{};
    mavlink_msg_command_long_decode(&message,&cmd);
    if (cmd.target_system!=m_sys_id || cmd.target_component!=m_comp_id || cmd.command!=MAV_CMD_REQUEST_MESSAGE ||
        (cmd.param1!=MAVLINK_MSG_ID_COMPONENT_INFORMATION && cmd.param1!=397)) return false;
    refresh_metadata();
    char uri[100]{}; std::strcpy(uri,"mftp:///component_metadata/general.json");
    char empty[100]{};
    mavlink_message_t out{};
    const auto crc=openhd::settings::crc32(m_metadata_ftp.files.at("/component_metadata/general.json"));
    if (cmd.param1==397) {
      // Standard COMPONENT_METADATA (common.xml ID 397, CRC extra 182).
      // The bundled dialect predates this replacement for COMPONENT_INFORMATION.
      // Pack its published wire layout without changing the dialect/submodule.
      char payload[108]{};
      _mav_put_uint32_t(payload,0,0); _mav_put_uint32_t(payload,4,crc);
      _mav_put_char_array(payload,8,uri,100);
      std::memcpy(_MAV_PAYLOAD_NON_CONST(&out),payload,sizeof(payload));
      out.msgid=397;
      mavlink_finalize_message(&out,m_sys_id,m_comp_id,108,108,182);
    } else {
      mavlink_msg_component_information_pack(m_sys_id,m_comp_id,&out,0,crc,uri,0,empty);
    }
    _sender->send_message(out);
    mavlink_msg_command_ack_pack(m_sys_id,m_comp_id,&out,MAV_CMD_REQUEST_MESSAGE,MAV_RESULT_ACCEPTED,0,0,message.sysid,message.compid);
    _sender->send_message(out);
    return true;
  }
  return false;
}

void XMavlinkParamProvider::poll_configuration_file() {
  if (!m_ready || std::chrono::steady_clock::now()-m_last_configuration_check<std::chrono::seconds(1)) return;
  m_last_configuration_check=std::chrono::steady_clock::now();
  const char* override_path=std::getenv("OPENHD_RUNTIME_SETTINGS_FILE");
  const std::string path=override_path ? override_path :
    (std::filesystem::exists("/Config/openhd/runtime-settings.json") ? "/Config/openhd/runtime-settings.json" : "/boot/openhd/runtime-settings.json");
  try {
    if (!std::filesystem::exists(path) || std::filesystem::file_size(path)>1024*1024) return;
    std::ifstream input(path);
    const std::string content((std::istreambuf_iterator<char>(input)),{});
    if (content==m_last_configuration) return;
    m_last_configuration=content; // Rejections are reported once; editing retries.
    const auto doc=nlohmann::json::parse(content);
    if (doc.at("version")!=1 || doc.at("systemId")!=m_sys_id) return;
    const auto key=std::to_string(m_comp_id);
    if (!doc.at("components").contains(key)) return;
    const auto& values=doc.at("components").at(key);
    if (!values.is_object()) throw std::runtime_error("Component settings must be an object");
    auto results=nlohmann::json::object();
    for (auto it=values.begin();it!=values.end();++it) {
      const auto found=std::find_if(m_registered_settings.begin(),m_registered_settings.end(),[&](const auto& s){return s.id==it.key();});
      bool accepted=false;
      if (found!=m_registered_settings.end() && openhd::settings::valid(*found,it.value())) {
        if (std::holds_alternative<openhd::IntSetting>(found->setting)) {
          const auto& s=std::get<openhd::IntSetting>(found->setting);
          const auto n=it.value().get<int>();
          accepted=s.change_callback && s.change_callback(found->id,n);
          if (accepted) _mavlink_parameter_receiver->update_existing_server_param_int(found->id,s.get_callback ? s.get_callback() : n);
        } else {
          const auto& s=std::get<openhd::StringSetting>(found->setting);
          const auto value=it.value().get<std::string>();
          accepted=s.change_callback && s.change_callback(found->id,value);
          if (accepted) _mavlink_parameter_receiver->update_existing_server_param_custom(found->id,s.get_callback ? s.get_callback() : value);
        }
      }
      results[it.key()]={{"accepted",accepted},{"requested",it.value()}};
      if (!accepted) openhd::log::get_default()->warn("Runtime settings file rejected {} on component {}",it.key(),m_comp_id);
    }
    // Read-only diagnostic report is also available to remote MAVLink clients.
    m_metadata_ftp.files["/openhd/file-results.json"]=nlohmann::json{
      {"version",1},{"systemId",m_sys_id},{"componentId",m_comp_id},{"results",results}}.dump();
  } catch (const std::exception& e) {
    openhd::log::get_default()->warn("Invalid runtime settings file: {}",e.what());
  }
}

std::vector<MavlinkMessage> XMavlinkParamProvider::process_mavlink_messages(
    std::vector<MavlinkMessage> messages) {
  std::lock_guard<std::mutex> lock(_mutex);
  for (const auto& setting : m_int_settings_with_update_functionality) {
    const auto intSetting = std::get<openhd::IntSetting>(setting.setting);
    const auto currValue =
        _mavlink_parameter_receiver->retrieve_server_param_int(setting.id);
    if (currValue.first == mavsdk::MavlinkParameterReceiver::Result::Success) {
      const int currValueInt = currValue.second;
      const int newIntvalue = intSetting.get_callback();
      if (currValueInt != newIntvalue) {
        // Param set on ground is now different to the one inside the gcs
        openhd::log::get_default()->warn("Updating {} from {} to {}",
                                         setting.id, currValueInt, newIntvalue);
        _mavlink_parameter_receiver->update_existing_server_param_int(
            setting.id, newIntvalue);
      }
    }
  }
  for (const auto& msg : messages) {
    if (process_metadata(msg.m)) continue;
    // One parameter write can change another setting (e.g. resolution resets
    // the sensor mode). Refresh before every request, including batched writes.
    for (const auto& setting : m_string_settings_with_update_functionality) {
      const auto& string_setting = std::get<openhd::StringSetting>(setting.setting);
      const auto current =
          _mavlink_parameter_receiver->retrieve_server_param_custom(setting.id);
      const auto value = string_setting.get_callback();
      if (current.first == mavsdk::MavlinkParameterReceiver::Result::Success &&
          current.second != value) {
        _mavlink_parameter_receiver->update_existing_server_param_custom(
            setting.id, value);
      }
    }
    _mavlink_message_handler->process_message(msg.m);
  }
  for (int i = 0; i < 100; i++) {
    _mavlink_parameter_receiver->do_work();
  }
  auto msges = _sender->messages;
  // std::cout<<"XMavlinkParamProvider::process_mavlink_message:"<<msges.size()<<"\n";
  _sender->messages.clear();
  return msges;
}

std::optional<MavlinkMessage> XMavlinkParamProvider::broadcast_read_only_string(const std::string& id) {
  std::lock_guard<std::mutex> lock(_mutex);
  if (!m_ready) return std::nullopt;
  for (size_t i = 0; i < m_registered_settings.size(); ++i) {
    const auto& setting = m_registered_settings[i];
    if (setting.id != id || !setting.read_only ||
        !std::holds_alternative<openhd::StringSetting>(setting.setting)) continue;
    const auto& value = std::get<openhd::StringSetting>(setting.setting);
    const auto text = value.get_callback ? value.get_callback() : value.value;
    char param_id[16]{}; char param_value[128]{};
    std::copy_n(id.data(), std::min(id.size(), sizeof(param_id)), param_id);
    std::copy_n(text.data(), std::min(text.size(), sizeof(param_value)), param_value);
    MavlinkMessage message{};
    mavlink_msg_param_ext_value_pack(m_sys_id, m_comp_id, &message.m, param_id,
        param_value, MAV_PARAM_EXT_TYPE_CUSTOM, m_registered_settings.size(), i);
    return message;
  }
  return std::nullopt;
}

std::vector<MavlinkMessage> XMavlinkParamProvider::generate_mavlink_messages() {
  std::lock_guard<std::mutex> lock(_mutex);
  poll_configuration_file();
  std::vector<MavlinkMessage> ret;
  if (m_opt_heartbeat_interval.has_value()) {
    const auto now = std::chrono::steady_clock::now();
    const auto elapsed = now - m_last_heartbeat;
    if (elapsed > m_opt_heartbeat_interval.value()) {
      m_last_heartbeat = now;
      ret.push_back(MavlinkComponent::create_heartbeat());
    }
  }
  return ret;
}
