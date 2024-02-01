#include "mavlink_parameter_subscription.h"

namespace mavsdk {

template <class T>
void MavlinkParameterSubscription::subscribe_param_changed(
    const std::string& name, const ParamChangedCallback<T>& callback,
    const void* cookie) {
  std::lock_guard<std::mutex> lock(_param_changed_subscriptions_mutex);
  if (callback != nullptr) {
    ParamChangedSubscription subscription{name, callback, cookie};
    // This is just to let the upper level know of what probably is a bug, but
    // we check again when actually calling the callback We also cannot assume
    // here that the user called provide_param before subscribe_param_changed,
    // so the only thing that makes sense is to log a warning, but then continue
    // anyways.
    /*std::lock_guard<std::mutex> lock2(_all_params_mutex);
    if (_all_params.find(name) != _all_params.end()) {
        const auto curr_value = _all_params.at(name);
        if (!curr_value.template is_same_type_templated<T>()) {
            LogDebug()
                << "You just registered a param changed callback where the type
    does not match the type already stored";
        }
    }*/
    _param_changed_subscriptions.push_back(subscription);
  } else {
    for (auto it = _param_changed_subscriptions.begin();
         it != _param_changed_subscriptions.end();
         /* ++it */) {
      if (it->param_name == name && it->cookie == cookie) {
        it = _param_changed_subscriptions.erase(it);
      } else {
        ++it;
      }
    }
  }
}

void MavlinkParameterSubscription::subscribe_param_float_changed(
    const std::string& name, const ParamFloatChangedCallback& callback,
    const void* cookie) {
  subscribe_param_changed<float>(name, callback, cookie);
}

void MavlinkParameterSubscription::subscribe_param_int_changed(
    const std::string& name, const ParamIntChangedCallback& callback,
    const void* cookie) {
  subscribe_param_changed<int>(name, callback, cookie);
}

void MavlinkParameterSubscription::subscribe_param_custom_changed(
    const std::string& name, const ParamCustomChangedCallback& callback,
    const void* cookie) {
  subscribe_param_changed<std::string>(name, callback, cookie);
}

void MavlinkParameterSubscription::find_and_call_subscriptions_value_changed(
    const std::string& param_name, const ParamValue& value) {
  std::lock_guard<std::mutex> lock(_param_changed_subscriptions_mutex);
  for (const auto& subscription : _param_changed_subscriptions) {
    if (subscription.param_name != param_name) {
      continue;
    }
    // We have a subscription on this param name, now check if the subscription
    // is for the right type and call the callback when matching
    if (std::get_if<ParamFloatChangedCallback>(&subscription.callback) &&
        value.get_float()) {
      std::get<ParamFloatChangedCallback>(subscription.callback)(
          value.get_float().value());
    } else if (std::get_if<ParamIntChangedCallback>(&subscription.callback) &&
               value.get_int()) {
      std::get<ParamIntChangedCallback>(subscription.callback)(
          value.get_int().value());
    } else if (std::get_if<ParamCustomChangedCallback>(
                   &subscription.callback) &&
               value.get_custom()) {
      std::get<ParamCustomChangedCallback>(subscription.callback)(
          value.get_custom().value());
    } else {
      // The callback we have set is not for this type.
      LogErr() << "Type and callback mismatch";
    }
  }
}

}  // namespace mavsdk