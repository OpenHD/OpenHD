#pragma once

#include <list>
#include <map>
#include <string>
#include <utility>

#include "locked_queue.h"
#include "mavlink_message_handler.h"
#include "mavlink_parameter_set.h"
#include "mavlink_parameter_subscription.h"
#include "param_value.h"
#include "sender.h"

namespace mavsdk {

/**
 * One could also call this MavlinkParameterProvider or MavlinkParameterServer
 * but all these names don't really fit perfectly. This class makes it easy to
 * accomplish the following task: Having a component that exposes some settings
 * for a user to configure. In General, the usage of this class is as following:
 * 1) provide parameters (now other components can request and change these
 * parameters via mavlink). 2) add listener(s) for these parameters such that
 * you can react to changes set by another component. 3) let mavlink do its
 * magic. Note that this side is much more simple - it does not need to worry
 * about re-transmission or such, the responsibility for that lies completely on
 * the part that wants to change parameters. Regarding non-extended and extended
 * parameters protocol: In addition to the parameter types from the non-extended
 * parameter protocol, the extended parameter protocol also supports string
 * parameter values. This class supports clients using both the non-extended and
 * the extended parameter protocol, but hides the string parameter values from
 * non-extended protocol clients. Therefore, if the server has std:.string
 * parameters but is talking to a non-extended client, param_index and
 * param_count are different compared to talking to a client who doesn't speak
 * extended.
 */
class MavlinkParameterReceiver {
 public:
  MavlinkParameterReceiver() = delete;
  explicit MavlinkParameterReceiver(
      Sender& parent,
      MavlinkMessageHandler& message_handler);  //,
  // by providing all the parameters on construction you can populate the
  // parameter set before the server starts reacting to clients, removing this
  // issue: https://mavlink.io/en/services/parameter.html#parameters_invariant
  // std::optional<std::map<std::string,ParamValue>>
  // optional_param_values=std::nullopt);
  ~MavlinkParameterReceiver();

  enum class Result {
    Success,            // All Ok
    WrongType,          // Wrong type provided
    ParamNameTooLong,   // param name provided too long
    NotFound,           // param not found
    ParamValueTooLong,  // value for param of type string doesn't fit into
                        // extended protocol.
  };

  /**
   * Add a new parameter to the parameter set.
   * It is recommended to not change the parameter set after the first
   * communication with any client.
   * (https://mavlink.io/en/services/parameter_ext.html#parameters_invariant).
   * @param name the unique id for this parameter
   * @param param_value the value for this parameter
   * @return Result::ParamNameTooLong if the parameter name is too long,
   * Result::WrongType if the same parameter name is provided with a different
   * type (aka updating the parameter would mutate the type of an already
   * provided parameter), Result::ParamValueTooLong if the parameter type is
   * std::string but the value is longer than the extended protocol allows and
   * Result::Success otherwise.
   */
  // Result provide_server_param(const std::string& name,const ParamValue&
  // param_value);
  template <class T>
  Result provide_server_param(
      const std::string& name, const T& param_value,
      std::function<bool(std::string id, T requested_value)> change_callback =
          nullptr);
  // convenient implementations for the 3 most commonly used types
  Result provide_server_param_float(const std::string& name, float value);
  Result provide_server_param_int(const std::string& name, int32_t value);
  Result provide_server_param_custom(const std::string& name,
                                     const std::string& value);

  /**
   * NOTE: Calling this method should be avoided - it results in an invariant
   * parameter set. However, sometimes this cannot be avoided
   */
  template <class T>
  Result update_existing_server_param(const std::string& name,
                                      const T& param_value);
  Result update_existing_server_param_int(const std::string& name,
                                          const int param_value);

  /**
   * @return a copy of the current parameter set of the server.
   */
  std::map<std::string, ParamValue> retrieve_all_server_params();

  /**
   * Retrieve the current value for a parameter from the server parameter set.
   * @tparam T the type of the parameter to retrieve, if the parameter from the
   * parameter set does not match this type, the method will return
   * MAVLinkParameters::Result::WrongType  and the value is default constructed.
   * @param name the name of the parameter to retrieve, if the parameter set
   * does not contain this name key MAVLinkParameters::Result::NotFound is
   * returned and the value is default constructed
   * @return MAVLinkParameters::Result::Success if the name is a valid key for
   * the parameter set, AND the type matches the value in the set. Otherwise,one
   * of the error codes above.
   */
  template <class T>
  std::pair<Result, T> retrieve_server_param(const std::string& name);
  std::pair<Result, float> retrieve_server_param_float(const std::string& name);
  std::pair<Result, int32_t> retrieve_server_param_int(const std::string& name);
  std::pair<Result, std::string> retrieve_server_param_custom(
      const std::string& name);

  void do_work();

  friend std::ostream& operator<<(std::ostream&, const Result&);

  // Non-copyable
  MavlinkParameterReceiver(const MavlinkParameterReceiver&) = delete;
  const MavlinkParameterReceiver& operator=(const MavlinkParameterReceiver&) =
      delete;

  void ready_for_communication();

 private:
  bool ready = false;
  /**
   * internally process a param set, coming from either the extended or
   * non-extended protocol. This checks and properly handles the following
   * conditions: 1) weather the param is inside the parameter set 2) weather the
   * type of the param inside the parameter set matches the type from the
   * request 3) TODO what to do if the value to set matches the current value.
   * @param param_id the id of the parameter in the set request message
   * @param value the value obtained from the set request message
   * @param extended true if the message is coming from the extended
   * protocol,false otherwise. The response workflow is slightly different on
   * the extended protocol.
   */
  void process_param_set_internally(const std::string& param_id,
                                    const ParamValue& value_to_set,
                                    bool extended);
  void process_param_set(const mavlink_message_t& message);
  void process_param_ext_set(const mavlink_message_t& message);

  Sender& _sender;
  MavlinkMessageHandler& _message_handler;

  std::mutex _all_params_mutex{};
  MavlinkParameterSet _param_set;

  // response: broadcast a specific parameter if found, ignores string
  // parameters
  void process_param_request_read(const mavlink_message_t& message);
  //  response: broadcast a specific parameter if found
  void process_param_ext_request_read(const mavlink_message_t& message);
  // send the appropriate response on a read request with a valid identifier.
  // (weather this parameter then exists is still unchecked)
  void internal_process_param_request_read(
      const std::variant<std::string, uint16_t>& identifier, bool extended);
  //  response: broadcast all parameters, ignores string parameters
  void process_param_request_list(const mavlink_message_t& message);
  //  response: broadcast all parameters
  void process_param_ext_request_list(const mavlink_message_t& message);
  // broadcast all current parameters. If extended=false, string parameters are
  // ignored.
  void broadcast_all_parameters(bool extended);

  // These are specific depending on the work item type.
  // note that ack needs fewer arguments.
  // Emitted on a get value or set value for non-extended, broadcast the current
  // value
  struct WorkItemValue {
    const uint16_t param_index;
    const uint16_t param_count;
    const bool extended;
  };
  // Emitted on a set value for the extended protocol only
  struct WorkItemAck {
    const PARAM_ACK param_ack;
  };
  // On the server side, the only benefit of using the work item pattern (mavsdk
  // specific) is that the request all parameters command(s) are less likely to
  // saturate the link.
  struct WorkItem {
    // A response always has a valid param id
    const std::string param_id;
    // as well as a valid param value
    const ParamValue param_value;
    using WorkItemVariant = std::variant<WorkItemValue, WorkItemAck>;
    const WorkItemVariant work_item_variant;
    explicit WorkItem(std::string param_id1, ParamValue param_value1,
                      WorkItemVariant work_item_variant1)
        : param_id(std::move(param_id1)),
          param_value(std::move(param_value1)),
          work_item_variant(std::move(work_item_variant1)){};
  };
  LockedQueue<WorkItem> _work_queue{};
  /**
   * See:
   * https://mavlink.io/en/services/parameter.html#multi-system-and-multi-component-support
   * @return true if the message should be processed by this server, false
   * otherwise.
   * @param target_sys_id the target sys id from the mavlink param message
   * @param target_comp_id the target component id from the mavlink param
   * message
   * @param is_request we also respond to MAV_COMP_ID_ALL on messages that are a
   * "request", but not on the "set" messages.
   */
  bool target_matches(uint16_t target_sys_id, uint16_t target_comp_id,
                      bool is_request);
  void log_target_mismatch(uint16_t target_sys_id, uint16_t target_comp_id);

  // Helper for safely handling a request_read or ext_request_read message
  // (which have the exact same layout). returns the identifier that should be
  // used or nothing if the message is ill-formed. See
  // https://mavlink.io/en/messages/common.html#PARAM_REQUEST_READ and
  // https://mavlink.io/en/messages/common.html#PARAM_EXT_REQUEST_READ
  static std::optional<std::variant<std::string, std::uint16_t>>
  extract_request_read_param_identifier(int16_t param_index,
                                        const char* param_id);
  const bool enable_log_target_mismatch = false;
  // not following the mavlink standard
  // have a minimum delay in between "broadcast all param(s)" requests
  std::chrono::steady_clock::time_point m_last_broadcast_all_request =
      std::chrono::steady_clock::now();
};

}  // namespace mavsdk