//
// Created by consti10 on 18.07.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_SETTINGS_PERSISTENT_HPP_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_SETTINGS_PERSISTENT_HPP_

#include <cassert>
#include <fstream>
#include <functional>
#include <utility>

#include "openhd_spdlog.h"
#include "openhd_util_filesystem.h"

/**
 * In general, all OpenHD modules (e.g. video, telemetry, interface) handle
 * their settings completely independently by writing and reading json files.
 */
namespace openhd {

/**
 * Helper class to persist settings during reboots (impl is using most likely
 * json in OpenHD). Properly handles the typical edge cases, e.g. a) No settings
 * have been stored for the given unique filename (e.g. for camera of type X) =>
 * create default settings. b) The user/developer manually wrote values of the
 * wrong type into the json file => delete invalid settings, create default.
 * This class is a bit hard to understand, I'd recommend just looking up one of
 * the implementations to understand it.
 * @tparam T the settings struct to persist
 */
template <class T>
class PersistentSettings {
 public:
  /**
   * @param base_path the directory into which the settings file is then
   * written. (filename: base_path+unique filename).
   */
  explicit PersistentSettings(std::string base_path)
      : _base_path(std::move(base_path)) {
    assert(_base_path.back() == '/');
  }
  // delete copy and move constructor
  PersistentSettings(const PersistentSettings&) = delete;
  PersistentSettings(const PersistentSettings&&) = delete;
  /**
   * read only, to express the need for calling persist otherwise
   */
  [[nodiscard]] const T& get_settings() const {
    assert(_settings);
    return *_settings;
  }
  /**
   * Don't forget to call persist once done modifying
   * @return
   */
  [[nodiscard]] T& unsafe_get_settings() const {
    assert(_settings);
    return *_settings;
  }
  // save changes by writing them out to the file, and notifying the listener cb
  // if there is any
  void persist(bool trigger_restart = true) const {
    PersistentSettings::persist_settings();
    if (_settings_changed_callback && trigger_restart) {
      _settings_changed_callback();
    }
  }
  // Persist then new settings, then call the callback to propagate the change
  void update_settings(const T& new_settings) {
    openhd::log::debug_log("Got new settings in [" + get_unique_filename() +
                           "]");
    _settings = std::make_unique<T>(new_settings);
    PersistentSettings::persist_settings();
    if (_settings_changed_callback) {
      _settings_changed_callback();
    }
  }
  typedef std::function<void()> SETTINGS_CHANGED_CALLBACK;
  void register_listener(SETTINGS_CHANGED_CALLBACK callback) {
    assert(!_settings_changed_callback);
    _settings_changed_callback = std::move(callback);
  }
  /*
   * looks for a previously written file (base_path+unique filename).
   * If this file exists, create settings from it - otherwise, create default
   * and persist.
   */
  void init() {
    if (!OHDFilesystemUtil::exists(_base_path)) {
      OHDFilesystemUtil::create_directory(_base_path);
    }
    const auto last_settings_opt = read_last_settings();
    if (last_settings_opt.has_value()) {
      _settings = std::make_unique<T>(last_settings_opt.value());
      openhd::log::info_log("Using settings in [" + get_file_path() + "]");
    } else {
      openhd::log::info_log("Creating default settings in [" + get_file_path() +
                            "]");
      // create default settings and persist them for the next reboot
      _settings = std::make_unique<T>(create_default());
      persist_settings();
    }
  }

 protected:
  // NEEDS TO BE OVERRIDDEN
  [[nodiscard]] virtual std::string get_unique_filename() const = 0;
  virtual T create_default() const = 0;
  virtual std::optional<T> impl_deserialize(
      const std::string& file_as_string) const = 0;
  virtual std::string imp_serialize(const T& data) const = 0;

 private:
  const std::string _base_path;
  std::unique_ptr<T> _settings;
  SETTINGS_CHANGED_CALLBACK _settings_changed_callback = nullptr;
  [[nodiscard]] std::string get_file_path() const {
    return _base_path + get_unique_filename();
  }
  /**
   * serialize settings to json and write to file for persistence
   */
  void persist_settings() const {
    assert(_settings);
    const auto file_path = get_file_path();
    // Serialize, then write to file
    const auto content = imp_serialize(*_settings);
    OHDFilesystemUtil::write_file(file_path, content);
  }
  /**
   * Try and deserialize the last stored settings (json)
   * Return std::nullopt if
   * 1) The file does not exist
   *  2) The json parse encountered an error
   *  3) The json conversion encountered an error
   *  In case of 1 this is most likely new hw, and default settings will be
   * created. In case of 2,3 it was most likely a user that modified the json
   * incorrectly Also, default settings will be created in this case.
   */
  [[nodiscard]] std::optional<T> read_last_settings() const {
    const auto file_path = get_file_path();
    const auto opt_content = OHDFilesystemUtil::opt_read_file(file_path);
    if (!opt_content.has_value()) {
      return std::nullopt;
    }
    const std::string content = opt_content.value();
    const auto parsed_opt = impl_deserialize(content);
    return parsed_opt;
  }
};

}  // namespace openhd

#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_SETTINGS_PERSISTENT_HPP_
