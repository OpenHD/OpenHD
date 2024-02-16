#include "mavlink_parameter_set.h"

#include <utility>

namespace mavsdk {

bool MavlinkParameterSet::add_new_parameter(
    const std::string &param_id, ParamValue value,
    std::function<bool(std::string id, ParamValue requested_value)>
        change_callback) {
  std::lock_guard<std::mutex> lock(_all_params_mutex);
  if (!validate_param_id(param_id)) {
    if (enable_debugging) {
      LogDebug() << "Invalid param_id:{" << param_id << "}";
    }
    return false;
  }
  if (_param_id_to_idx.find(param_id) != _param_id_to_idx.end()) {
    // this parameter does already exist, we cannot add it as a new one.
    return false;
  }
  if (_all_params.size() + 1 > MAX_N_PARAMETERS) {
    // not enough space for this parameter
    return false;
  }
  InternalParameter parameter{param_id, std::move(value),
                              std::move(change_callback)};
  _all_params.push_back(parameter);
  // just don't think about it.
  _param_index_to_hidden_extended.push_back(param_count_non_extended);
  _param_id_to_idx[param_id] = static_cast<uint16_t>(_param_id_to_idx.size());
  if (!parameter.value.needs_extended()) {
    param_count_non_extended++;
  }
  if (enable_debugging) {
    LogDebug() << "Added parameter: " << parameter;
  }
  assert(_all_params.size() == _param_index_to_hidden_extended.size());
  assert(_all_params.size() == _param_id_to_idx.size());
  return true;
}

MavlinkParameterSet::UpdateExistingParamResult
MavlinkParameterSet::update_existing_parameter(const std::string &param_id,
                                               const ParamValue &value) {
  std::lock_guard<std::mutex> lock(_all_params_mutex);
  if (_param_id_to_idx.find(param_id) == _param_id_to_idx.end()) {
    // this parameter does not exist yet.
    LogDebug() << "MavlinkParameterSet::update_existing_parameter " << param_id
               << " does not exist";
    return UpdateExistingParamResult::MISSING_PARAM;
  }
  const auto index = _param_id_to_idx.at(param_id);
  const auto parameter = _all_params.at(index);
  if (!parameter.value.is_same_type(value)) {
    // We cannot mutate the parameter type.
    LogDebug() << "Cannot mutate the type of " << param_id << " from "
               << parameter.value.typestr() << " to " << value.typestr();
    return UpdateExistingParamResult::WRONG_PARAM_TYPE;
  }
  if (parameter.value == value) {
    return UpdateExistingParamResult::NO_CHANGE;
  }
  if (parameter.change_callback) {
    const auto before = std::chrono::steady_clock::now();
    const auto result = parameter.change_callback(param_id, value);
    const auto time_spend_on_user_callback =
        std::chrono::steady_clock::now() - before;
    if (time_spend_on_user_callback > std::chrono::seconds(1)) {
      LogWarn() << "Spent more than 1 second on param user callback for "
                << param_id;
    }
    if (!result) {
      // Param was rejected by returning false in the change callback
      return UpdateExistingParamResult::REJECTED;
    }
  }
  _all_params.at(index).value.update_value_typesafe(value);
  return UpdateExistingParamResult::SUCCESS;
}

std::vector<MavlinkParameterSet::Parameter>
MavlinkParameterSet::list_all_parameters(const bool supports_extended) {
  std::lock_guard<std::mutex> lock(_all_params_mutex);
  std::vector<MavlinkParameterSet::Parameter> ret;
  uint16_t index = 0;
  for (const auto &param : _all_params) {
    if (param.value.needs_extended() && !supports_extended) {
      continue;
    }
    ret.emplace_back(
        MavlinkParameterSet::Parameter{param.param_id, index, param.value});
    index++;
  }
  return ret;
}

std::map<std::string, ParamValue> MavlinkParameterSet::create_copy_as_map() {
  std::lock_guard<std::mutex> lock(_all_params_mutex);
  std::map<std::string, ParamValue> ret;
  for (const auto &param : _all_params) {
    ret[param.param_id] = param.value;
  }
  return ret;
}

uint16_t MavlinkParameterSet::get_current_parameters_count(bool extended) {
  std::lock_guard<std::mutex> lock(_all_params_mutex);
  if (extended) {
    // easy, we can do all parameters.
    return static_cast<uint16_t>(_all_params.size());
  }
  return param_count_non_extended;
}

std::optional<MavlinkParameterSet::Parameter>
MavlinkParameterSet::lookup_parameter(const std::string &param_id,
                                      bool extended) {
  std::lock_guard<std::mutex> lock(_all_params_mutex);
  if (_param_id_to_idx.find(param_id) == _param_id_to_idx.end()) {
    // param does not exist
    return {};
  }
  const auto param_index = _param_id_to_idx.at(param_id);
  const auto param = _all_params.at(param_index);
  if (param.value.needs_extended() && !extended) {
    // param exists, but needs extended
    return {};
  }
  const auto param_index_actual =
      extended ? param_index : _param_index_to_hidden_extended.at(param_index);
  return MavlinkParameterSet::Parameter{param.param_id, param_index_actual,
                                        param.value};
}

std::optional<MavlinkParameterSet::Parameter>
MavlinkParameterSet::lookup_parameter(const uint16_t param_index,
                                      bool extended) {
  std::lock_guard<std::mutex> lock(_all_params_mutex);
  if (param_index >= _all_params.size()) {
    // param des not exist
    return {};
  }
  const auto param = _all_params.at(param_index);
  if (param.value.needs_extended() && !extended) {
    // param exists, but needs extended
    return {};
  }
  const auto param_index_actual =
      extended ? param_index : _param_index_to_hidden_extended.at(param_index);
  return MavlinkParameterSet::Parameter{param.param_id, param_index_actual,
                                        param.value};
}

std::optional<MavlinkParameterSet::Parameter>
MavlinkParameterSet::lookup_parameter(
    const MavlinkParameterSet::ParamIdentifier &identifier, bool extended) {
  if (std::holds_alternative<std::string>(identifier)) {
    const auto param_id = std::get<std::string>(identifier);
    return lookup_parameter(param_id, extended);
  }
  return lookup_parameter(std::get<std::uint16_t>(identifier), extended);
}

std::string MavlinkParameterSet::param_identifier_to_string(
    const MavlinkParameterSet::ParamIdentifier &param_identifier) {
  std::stringstream ss;
  ss << "ParamIdentifier:{";
  if (std::holds_alternative<std::string>(param_identifier)) {
    ss << "str:" << std::get<std::string>(param_identifier);
  } else {
    ss << "int:" << (int)std::get<std::uint16_t>(param_identifier);
  }
  ss << "}";
  return ss.str();
}

std::ostream &operator<<(
    std::ostream &strm,
    const MavlinkParameterSet::UpdateExistingParamResult &obj) {
  switch (obj) {
    case MavlinkParameterSet::UpdateExistingParamResult::MISSING_PARAM:
      strm << "MISSING_PARAM";
      break;
    case MavlinkParameterSet::UpdateExistingParamResult::SUCCESS:
      strm << "SUCCESS";
      break;
    case MavlinkParameterSet::UpdateExistingParamResult::WRONG_PARAM_TYPE:
      strm << "WRONG_PARAM_TYPE";
      break;
  }
  return strm;
}

std::string MavlinkParameterSet::extract_safe_param_id(const char *param_id) {
  // The param_id field of the MAVLink struct has length 16 and can not be null
  // terminated. Therefore, we make a 0 terminated copy first.
  char param_id_long_enough[PARAM_ID_LEN + 1] = {};
  std::memcpy(param_id_long_enough, param_id, PARAM_ID_LEN);
  return {param_id_long_enough};
}

std::array<char, MavlinkParameterSet::PARAM_ID_LEN>
MavlinkParameterSet::param_id_to_message_buffer(const std::string &param_id) {
  assert(param_id.length() <= PARAM_ID_LEN);
  std::array<char, PARAM_ID_LEN> ret = {};
  std::memcpy(ret.data(), param_id.c_str(), param_id.length());
  return ret;
}
bool MavlinkParameterSet::validate_param_id(const std::string &param_id) {
  if (param_id.empty()) {
    return false;
  }
  if (param_id.size() > MavlinkParameterSet::PARAM_ID_LEN) {
    return false;
  }
  return true;
}

std::ostream &operator<<(std::ostream &strm,
                         const MavlinkParameterSet::InternalParameter &obj) {
  strm << "InternalParameter{id:(" << obj.param_id << ") value:("
       << obj.value.typestr() << "," << obj.value.get_string() << ")}";
  return strm;
}

std::ostream &operator<<(std::ostream &strm,
                         const MavlinkParameterSet::Parameter &obj) {
  strm << "Parameter{id:(" << obj.param_id << ":" << obj.param_index
       << ") value:(" << obj.value.typestr() << "," << obj.value.get_string()
       << ")}";
  return strm;
}
}  // namespace mavsdk