#include "mavlink_parameter_receiver.h"

#include <cassert>

namespace mavsdk {

MavlinkParameterReceiver::MavlinkParameterReceiver(
    Sender& sender, MavlinkMessageHandler& message_handler)
    : _sender(sender), _message_handler(message_handler) {
  // Populate the parameter set before the first communication, if provided by
  // the user.
  /*if(optional_param_values.has_value()){
      const auto& param_values=optional_param_values.value();
      for(const auto& [key,value] : param_values){
          const auto result= provide_server_param(key,value);
          if(result!=Result::Success){
              LogDebug()<<"Cannot add parameter:"<<key<<":"<<value<<" "<<result;
          }
      }
  }*/
}

MavlinkParameterReceiver::~MavlinkParameterReceiver() {
  _message_handler.unregister_all(this);
}

void MavlinkParameterReceiver::ready_for_communication() {
  if (ready) return;
  _message_handler.register_one(
      MAVLINK_MSG_ID_PARAM_SET,
      [this](const mavlink_message_t& message) { process_param_set(message); },
      this);

  _message_handler.register_one(
      MAVLINK_MSG_ID_PARAM_EXT_SET,
      [this](const mavlink_message_t& message) {
        process_param_ext_set(message);
      },
      this);

  _message_handler.register_one(
      MAVLINK_MSG_ID_PARAM_REQUEST_READ,
      [this](const mavlink_message_t& message) {
        process_param_request_read(message);
      },
      this);

  _message_handler.register_one(
      MAVLINK_MSG_ID_PARAM_REQUEST_LIST,
      [this](const mavlink_message_t& message) {
        process_param_request_list(message);
      },
      this);

  _message_handler.register_one(
      MAVLINK_MSG_ID_PARAM_EXT_REQUEST_READ,
      [this](const mavlink_message_t& message) {
        process_param_ext_request_read(message);
      },
      this);

  _message_handler.register_one(
      MAVLINK_MSG_ID_PARAM_EXT_REQUEST_LIST,
      [this](const mavlink_message_t& message) {
        process_param_ext_request_list(message);
      },
      this);
  ready = true;
}

template <class T>
MavlinkParameterReceiver::Result MavlinkParameterReceiver::provide_server_param(
    const std::string& name, const T& value,
    std::function<bool(std::string id, T requested_value)> change_callback) {
  if (name.size() > MavlinkParameterSet::PARAM_ID_LEN) {
    LogErr() << "Error: param name too long";
    return Result::ParamNameTooLong;
  }
  std::lock_guard<std::mutex> lock(_all_params_mutex);
  ParamValue param_value;
  param_value.set(value);
  if (param_value.is<std::string>()) {
    const auto s = param_value.get<std::string>();
    if (s.size() > sizeof(mavlink_param_ext_set_t::param_value)) {
      LogErr() << "Error: param value too long";
      return Result::ParamValueTooLong;
    }
  }
  auto tmp = [param_value, change_callback](std::string id,
                                            ParamValue changed_value) {
    assert(changed_value.is_same_type(param_value));
    if (change_callback == nullptr) {
      return true;
    }
    const T changed_value_typed = changed_value.get<T>();
    return change_callback(id, changed_value_typed);
  };
  // Param set makes sure we cannot add the same parameter more than once and
  // keeps the type safe
  if (_param_set.add_new_parameter(name, param_value, tmp)) {
    return Result::Success;
  }
  return Result::WrongType;
}

MavlinkParameterReceiver::Result
MavlinkParameterReceiver::provide_server_param_float(const std::string& name,
                                                     float value) {
  return provide_server_param<float>(name, value);
}

MavlinkParameterReceiver::Result
MavlinkParameterReceiver::provide_server_param_int(const std::string& name,
                                                   int32_t value) {
  return provide_server_param<int>(name, value);
}

MavlinkParameterReceiver::Result
MavlinkParameterReceiver::provide_server_param_custom(
    const std::string& name, const std::string& value) {
  return provide_server_param<std::string>(name, value);
}

std::map<std::string, ParamValue>
MavlinkParameterReceiver::retrieve_all_server_params() {
  std::lock_guard<std::mutex> lock(_all_params_mutex);
  return _param_set.create_copy_as_map();
}

template <class T>
std::pair<MavlinkParameterReceiver::Result, T>
MavlinkParameterReceiver::retrieve_server_param(const std::string& name) {
  std::lock_guard<std::mutex> lock(_all_params_mutex);
  const auto param_opt = _param_set.lookup_parameter(name, true);
  if (!param_opt.has_value()) {
    return {Result::NotFound, {}};
  }
  // This parameter exists, check its type
  const auto& param = param_opt.value();
  if (param.value.is<T>()) {
    return {Result::Success, param.value.get<T>()};
  }
  return {Result::WrongType, {}};
}

std::pair<MavlinkParameterReceiver::Result, float>
MavlinkParameterReceiver::retrieve_server_param_float(const std::string& name) {
  return retrieve_server_param<float>(name);
}

std::pair<MavlinkParameterReceiver::Result, std::string>
MavlinkParameterReceiver::retrieve_server_param_custom(
    const std::string& name) {
  return retrieve_server_param<std::string>(name);
}

std::pair<MavlinkParameterReceiver::Result, int32_t>
MavlinkParameterReceiver::retrieve_server_param_int(const std::string& name) {
  return retrieve_server_param<int32_t>(name);
}

void MavlinkParameterReceiver::process_param_set_internally(
    const std::string& param_id, const ParamValue& value_to_set,
    bool extended) {
  LogDebug() << "Param set request " << (extended ? "Ext" : "") << ": "
             << param_id << " with " << value_to_set;
  std::lock_guard<std::mutex> lock(_all_params_mutex);
  // for checking if the update actually changed the value
  const auto opt_before_update =
      _param_set.lookup_parameter(param_id, extended);
  const auto result =
      _param_set.update_existing_parameter(param_id, value_to_set);
  const auto param_count = _param_set.get_current_parameters_count(extended);
  LogDebug() << result;
  switch (result) {
    case MavlinkParameterSet::UpdateExistingParamResult::MISSING_PARAM: {
      // We do not allow clients to add a new parameter to the parameter set,
      // only to update existing parameters. In this case, we cannot even
      // respond with anything, since this parameter simply does not exist.
      LogWarn() << "Got param_set for non-existing parameter:" << param_id;
      // TODO it is ambiguous if we should send a PARAM_ACK_VALUE_UNSUPPORTED
      // for the extended protocol only in this case. However, I don't think
      // that is a good idea - since we then basically need to construct a valid
      // parameter with this param_id and hope the client ignores it.
      return;
    }
    case MavlinkParameterSet::UpdateExistingParamResult::WRONG_PARAM_TYPE: {
      // We broadcast the un-changed parameter type and value, non-extended and
      // extended work differently here
      const auto curr_param =
          _param_set.lookup_parameter(param_id, extended).value();
      assert(curr_param.param_index < param_count);
      LogWarn() << "Got param_set for existing value, but WRONG_PARAM_TYPE. "
                   "registered param: "
                << curr_param;
      if (extended) {
        auto new_work =
            std::make_shared<WorkItem>(curr_param.param_id, curr_param.value,
                                       WorkItemAck{PARAM_ACK_FAILED});
        _work_queue.push_back(new_work);
      } else {
        auto new_work = std::make_shared<WorkItem>(
            curr_param.param_id, curr_param.value,
            WorkItemValue{curr_param.param_index, param_count, extended});
        _work_queue.push_back(new_work);
      }
      return;
    }
    case MavlinkParameterSet::UpdateExistingParamResult::REJECTED: {
      // We broadcast the un-changed parameter type and value, non-extended and
      // extended work differently here
      const auto curr_param =
          _param_set.lookup_parameter(param_id, extended).value();
      assert(curr_param.param_index < param_count);
      LogWarn() << "Got param_set for existing value, but REJECTED. registered "
                   "param: "
                << curr_param;
      if (extended) {
        auto new_work = std::make_shared<WorkItem>(
            curr_param.param_id, curr_param.value,
            WorkItemAck{PARAM_ACK_VALUE_UNSUPPORTED});
        _work_queue.push_back(new_work);
      } else {
        auto new_work = std::make_shared<WorkItem>(
            curr_param.param_id, curr_param.value,
            WorkItemValue{curr_param.param_index, param_count, extended});
        _work_queue.push_back(new_work);
      }
      return;
    }
    case MavlinkParameterSet::UpdateExistingParamResult::SUCCESS:
    case MavlinkParameterSet::UpdateExistingParamResult::NO_CHANGE: {
      // Even if the param was not changed, the response from the server needs
      // to be sent anyways, multiple ground station(s) changing params can
      // result in out-of-date values
      const auto updated_parameter =
          _param_set.lookup_parameter(param_id, extended).value();
      if (result == MavlinkParameterSet::UpdateExistingParamResult::SUCCESS) {
        LogDebug() << "Got param_set SUCCESS:" << updated_parameter;
      } else {
        assert(result ==
               MavlinkParameterSet::UpdateExistingParamResult::NO_CHANGE);
        LogDebug() << "Got param_set NO_CHANGE:" << updated_parameter;
      }
      if (extended) {
        auto new_work = std::make_shared<WorkItem>(
            updated_parameter.param_id, updated_parameter.value,
            WorkItemAck{PARAM_ACK_ACCEPTED});
        _work_queue.push_back(new_work);
      } else {
        auto new_work = std::make_shared<WorkItem>(
            updated_parameter.param_id, updated_parameter.value,
            WorkItemValue{updated_parameter.param_index, param_count,
                          extended});
        _work_queue.push_back(new_work);
      }
    } break;
    default:
      assert(true);
  }
}

void MavlinkParameterReceiver::process_param_set(
    const mavlink_message_t& message) {
  mavlink_param_set_t set_request{};
  mavlink_msg_param_set_decode(&message, &set_request);
  if (!target_matches(set_request.target_system, set_request.target_component,
                      false)) {
    log_target_mismatch(set_request.target_system,
                        set_request.target_component);
    return;
  }
  const std::string safe_param_id =
      MavlinkParameterSet::extract_safe_param_id(set_request.param_id);
  if (!MavlinkParameterSet::validate_param_id(safe_param_id)) {
    LogWarn() << "Invalid Param Set ID Request {" << safe_param_id << "}";
    return;
  }
  ParamValue value_to_set;
  if (!value_to_set.set_from_mavlink_param_set_bytewise(set_request)) {
    // This should never happen, the type enum in the message is unknown.
    LogWarn() << "Invalid Param Set Request: " << safe_param_id;
    return;
  }
  process_param_set_internally(safe_param_id, value_to_set, false);
}

void MavlinkParameterReceiver::process_param_ext_set(
    const mavlink_message_t& message) {
  mavlink_param_ext_set_t set_request{};
  mavlink_msg_param_ext_set_decode(&message, &set_request);
  if (!target_matches(set_request.target_system, set_request.target_component,
                      false)) {
    log_target_mismatch(set_request.target_system,
                        set_request.target_component);
    return;
  }
  const std::string safe_param_id =
      MavlinkParameterSet::extract_safe_param_id(set_request.param_id);
  if (!MavlinkParameterSet::validate_param_id(safe_param_id)) {
    LogWarn() << "Invalid Param Set ID Request {" << safe_param_id << "}";
    return;
  }
  ParamValue value_to_set;
  if (!value_to_set.set_from_mavlink_param_ext_set(set_request)) {
    // This should never happen, the type enum in the message is unknown.
    LogWarn() << "Invalid Param Set ext Request: " << safe_param_id;
    return;
  }
  process_param_set_internally(safe_param_id, value_to_set, true);
}

void MavlinkParameterReceiver::process_param_request_read(
    const mavlink_message_t& message) {
  mavlink_param_request_read_t read_request{};
  mavlink_msg_param_request_read_decode(&message, &read_request);
  if (!target_matches(read_request.target_system, read_request.target_component,
                      true)) {
    log_target_mismatch(read_request.target_system,
                        read_request.target_component);
    return;
  }
  const auto opt_param_id_or_index = extract_request_read_param_identifier(
      read_request.param_index, read_request.param_id);
  if (opt_param_id_or_index == std::nullopt) {
    LogWarn() << "Ill-formed param_request_read message";
    return;
  }
  internal_process_param_request_read(opt_param_id_or_index.value(), false);
}

void MavlinkParameterReceiver::process_param_ext_request_read(
    const mavlink_message_t& message) {
  mavlink_param_ext_request_read_t read_request{};
  mavlink_msg_param_ext_request_read_decode(&message, &read_request);
  if (!target_matches(read_request.target_system, read_request.target_component,
                      true)) {
    log_target_mismatch(read_request.target_system,
                        read_request.target_component);
    return;
  }
  const auto opt_param_id_or_index = extract_request_read_param_identifier(
      read_request.param_index, read_request.param_id);
  if (opt_param_id_or_index == std::nullopt) {
    LogWarn() << "Ill-formed param_ext_request_read message";
    return;
  }
  internal_process_param_request_read(opt_param_id_or_index.value(), true);
}

void MavlinkParameterReceiver::internal_process_param_request_read(
    const std::variant<std::string, uint16_t>& identifier,
    const bool extended) {
  std::lock_guard<std::mutex> lock(_all_params_mutex);
  // look up the parameter in the parameter set by its identifier.
  const auto param_opt = _param_set.lookup_parameter(identifier, extended);
  if (!param_opt.has_value()) {
    LogDebug() << "Ignoring request_read message " << (extended ? "ext " : "")
               << "- value not found "
               << MavlinkParameterSet::param_identifier_to_string(identifier);
    return;
  }
  const auto& param = param_opt.value();
  const auto param_count = _param_set.get_current_parameters_count(extended);
  assert(param.param_index < param_count);
  auto new_work = std::make_shared<WorkItem>(
      param.param_id, param.value,
      WorkItemValue{param.param_index, param_count, extended});
  _work_queue.push_back(new_work);
}

void MavlinkParameterReceiver::process_param_request_list(
    const mavlink_message_t& message) {
  mavlink_param_request_list_t list_request{};
  mavlink_msg_param_request_list_decode(&message, &list_request);
  if (!target_matches(list_request.target_system, list_request.target_component,
                      true)) {
    log_target_mismatch(list_request.target_system,
                        list_request.target_component);
    return;
  }
  broadcast_all_parameters(false);
}

void MavlinkParameterReceiver::process_param_ext_request_list(
    const mavlink_message_t& message) {
  mavlink_param_ext_request_list_t ext_list_request{};
  mavlink_msg_param_ext_request_list_decode(&message, &ext_list_request);
  if (!target_matches(ext_list_request.target_system,
                      ext_list_request.target_component, true)) {
    log_target_mismatch(ext_list_request.target_system,
                        ext_list_request.target_component);
    return;
  }
  broadcast_all_parameters(true);
}

void MavlinkParameterReceiver::broadcast_all_parameters(const bool extended) {
  std::lock_guard<std::mutex> lock(_all_params_mutex);
  const auto elapsed =
      std::chrono::steady_clock::now() - m_last_broadcast_all_request;
  if (elapsed < std::chrono::seconds(1)) {
    return;
  }
  m_last_broadcast_all_request = std::chrono::steady_clock::now();
  const auto all_params = _param_set.list_all_parameters(extended);
  LogDebug() << "broadcast_all_parameters " << (extended ? "Ext" : "") << ": "
             << all_params.size();
  for (const auto& parameter : all_params) {
    LogDebug() << "sending param:" << parameter;
    auto new_work = std::make_shared<WorkItem>(
        parameter.param_id, parameter.value,
        WorkItemValue{parameter.param_index,
                      static_cast<uint16_t>(all_params.size()), extended});
    _work_queue.push_back(new_work);
  }
}

void MavlinkParameterReceiver::do_work() {
  LockedQueue<WorkItem>::Guard work_queue_guard(_work_queue);
  auto work = work_queue_guard.get_front();
  if (!work) {
    return;
  }
  const auto param_id_message_buffer =
      MavlinkParameterSet::param_id_to_message_buffer(work->param_id);
  mavlink_message_t mavlink_message;
  if (std::holds_alternative<WorkItemValue>(work->work_item_variant)) {
    const auto& specific = std::get<WorkItemValue>(work->work_item_variant);
    if (specific.extended) {
      const auto buf = work->param_value.get_128_bytes();
      // mavlink_msg_param_ext_value_encode()
      mavlink_msg_param_ext_value_pack(
          _sender.get_own_system_id(), _sender.get_own_component_id(),
          &mavlink_message, param_id_message_buffer.data(), buf.data(),
          work->param_value.get_mav_param_ext_type(), specific.param_count,
          specific.param_index);
    } else {
      float param_value;
      if (_sender.autopilot() == Sender::Autopilot::ArduPilot) {
        param_value = work->param_value.get_4_float_bytes_cast();
      } else {
        param_value = work->param_value.get_4_float_bytes_bytewise();
      }
      mavlink_msg_param_value_pack(
          _sender.get_own_system_id(), _sender.get_own_component_id(),
          &mavlink_message, param_id_message_buffer.data(), param_value,
          work->param_value.get_mav_param_type(), specific.param_count,
          specific.param_index);
    }
    if (!_sender.send_message(mavlink_message)) {
      LogErr() << "Error: Send message failed";
      work_queue_guard.pop_front();
      return;
    }
    work_queue_guard.pop_front();
  } else {
    const auto& specific = std::get<WorkItemAck>(work->work_item_variant);
    auto buf = work->param_value.get_128_bytes();
    mavlink_msg_param_ext_ack_pack(
        _sender.get_own_system_id(), _sender.get_own_component_id(),
        &mavlink_message, param_id_message_buffer.data(), buf.data(),
        work->param_value.get_mav_param_ext_type(), specific.param_ack);
    if (!_sender.send_message(mavlink_message)) {
      LogErr() << "Error: Send message failed";
      work_queue_guard.pop_front();
      return;
    }
    work_queue_guard.pop_front();
  }
}

std::ostream& operator<<(std::ostream& str,
                         const MavlinkParameterReceiver::Result& result) {
  switch (result) {
    case MavlinkParameterReceiver::Result::Success:
      return str << "Success";
    case MavlinkParameterReceiver::Result::WrongType:
      return str << "WrongType";
    case MavlinkParameterReceiver::Result::ParamNameTooLong:
      return str << "ParamNameTooLong";
    case MavlinkParameterReceiver::Result::NotFound:
      return str << "NotFound";
    case MavlinkParameterReceiver::Result::ParamValueTooLong:
      return str << ":ParamValueTooLong";
    default:
      return str << "UnknownError";
  }
}

bool MavlinkParameterReceiver::target_matches(const uint16_t target_sys_id,
                                              const uint16_t target_comp_id,
                                              bool is_request) {
  if (target_sys_id != _sender.get_own_system_id()) {
    return false;
  }
  if (is_request) {
    return target_comp_id == _sender.get_own_component_id() ||
           target_comp_id == MAV_COMP_ID_ALL;
  }
  return target_comp_id == _sender.get_own_component_id();
}

void MavlinkParameterReceiver::log_target_mismatch(uint16_t target_sys_id,
                                                   uint16_t target_comp_id) {
  if (enable_log_target_mismatch) {
    LogDebug() << "Ignoring message - wrong target id. Got:"
               << (int)target_sys_id << ":" << (int)target_comp_id
               << " Wanted:" << (int)_sender.get_own_system_id() << ":"
               << (int)_sender.get_own_component_id();
  }
}

std::optional<std::variant<std::string, std::uint16_t>>
MavlinkParameterReceiver::extract_request_read_param_identifier(
    int16_t param_index, const char* param_id) {
  if (param_index == -1) {
    // use param_id if index == -1
    const auto safe_param_id =
        MavlinkParameterSet::extract_safe_param_id(param_id);
    if (MavlinkParameterSet::validate_param_id(safe_param_id)) {
      return safe_param_id;
    }
    LogWarn() << "Message with param_index=-1 but no valid param id";
    return std::nullopt;
  } else {
    // if index is not -1, it should be a valid parameter index (>=0)
    if (param_index >= 0) {
      return static_cast<uint16_t>(param_index);
    }
    LogWarn() << "Param_index " << param_index
              << " cannot be a valid param index";
  }
  return std::nullopt;
}

template <class T>
MavlinkParameterReceiver::Result
MavlinkParameterReceiver::update_existing_server_param(const std::string& name,
                                                       const T& value) {
  if (name.size() > MavlinkParameterSet::PARAM_ID_LEN) {
    LogErr() << "Error: param name too long";
    return Result::ParamNameTooLong;
  }
  std::lock_guard<std::mutex> lock(_all_params_mutex);
  ParamValue param_value;
  param_value.set(value);
  auto res = _param_set.update_existing_parameter(name, param_value);
  if (res == MavlinkParameterSet::UpdateExistingParamResult::SUCCESS)
    return MavlinkParameterReceiver::Result::Success;
  return MavlinkParameterReceiver::Result::NotFound;
}

MavlinkParameterReceiver::Result
MavlinkParameterReceiver::update_existing_server_param_int(
    const std::string& name, const int param_value) {
  return update_existing_server_param<int>(name, param_value);
}

}  // namespace mavsdk