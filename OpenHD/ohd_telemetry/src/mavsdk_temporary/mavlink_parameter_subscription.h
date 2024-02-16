#pragma once

#include <functional>
#include <list>
#include <mutex>

#include "param_value.h"

namespace mavsdk {

// Util for exposing the functionality of subscribing to parameter changes
// Note: maybe it would be cleaner to also have the parameter set in here, and
// make this class A "MavlinkParameterSet" so to say where set_xxx properly
// calls the subscriptions. But for now, I don't want to go down that route yet
// since I don't know if that isn't too much of inheritance. NOTE: r.n the
// inherited class still needs to remember to call
// find_and_call_subscriptions_value_changed() When a value is changed.
class MavlinkParameterSubscription {
 public:
  template <class T>
  using ParamChangedCallback = std::function<void(T value)>;

  /**
   * Subscribe to changes on the parameter referenced by @param name.
   * If the value for this parameter changes, the given callback is called
   * provided that the expected type matches the actual type of the parameter.
   */
  template <class T>
  void subscribe_param_changed(const std::string& name,
                               const ParamChangedCallback<T>& callback,
                               const void* cookie);

  using ParamFloatChangedCallback = ParamChangedCallback<float>;
  void subscribe_param_float_changed(const std::string& name,
                                     const ParamFloatChangedCallback& callback,
                                     const void* cookie);
  using ParamIntChangedCallback = ParamChangedCallback<int>;
  void subscribe_param_int_changed(const std::string& name,
                                   const ParamIntChangedCallback& callback,
                                   const void* cookie);
  using ParamCustomChangedCallback = ParamChangedCallback<std::string>;
  void subscribe_param_custom_changed(
      const std::string& name, const ParamCustomChangedCallback& callback,
      const void* cookie);

 protected:
  /**
   * Find all the subscriptions for the given @param param_name,
   * check their type and call them when matching. This does not check if the
   * given param actually was changed, but it is safe to call with mismatching
   * types.
   */
  void find_and_call_subscriptions_value_changed(
      const std::string& param_name, const ParamValue& new_param_value);

 private:
  using ParamChangedCallbacks =
      std::variant<ParamFloatChangedCallback, ParamIntChangedCallback,
                   ParamCustomChangedCallback>;
  struct ParamChangedSubscription {
    const std::string param_name;
    const ParamChangedCallbacks callback;
    const void* const cookie;
    explicit ParamChangedSubscription(std::string param_name1,
                                      ParamChangedCallbacks callback1,
                                      const void* cookie1)
        : param_name(std::move(param_name1)),
          callback(std::move(callback1)),
          cookie(cookie1){};
  };
  std::mutex _param_changed_subscriptions_mutex{};
  std::list<ParamChangedSubscription> _param_changed_subscriptions{};
};

}  // namespace mavsdk