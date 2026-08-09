/******************************************************************************
 * OpenHD runtime plugin loader.
 ******************************************************************************/

#ifndef OPENHD_PLUGIN_MANAGER_H
#define OPENHD_PLUGIN_MANAGER_H

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "openhd_plugin.h"

namespace openhd {

class PluginManager {
 public:
  static PluginManager& instance();

  PluginManager(const PluginManager&) = delete;
  PluginManager& operator=(const PluginManager&) = delete;

  // Loads *.so files from the default path, or OPENHD_PLUGIN_PATH when set.
  // Missing directories and rejected plugins are non-fatal.
  std::size_t load_plugins(bool is_air);
  std::size_t load_plugins(bool is_air,
                           const std::vector<std::string>& search_paths);

  void notify_video_bitrate_changed(
      const openhd_plugin_video_bitrate_event& event) noexcept;
  void notify_video_settings_changed(
      const openhd_plugin_video_settings_event& event) noexcept;
  bool notify_camera_control(
      const openhd_plugin_camera_control_event& event) noexcept;
  void shutdown() noexcept;
  [[nodiscard]] std::size_t loaded_plugin_count() const noexcept;
  [[nodiscard]] bool is_plugin_loaded(std::string_view name) const noexcept;

 private:
  PluginManager();
  ~PluginManager();

  struct LoadedPlugin;
  std::vector<std::unique_ptr<LoadedPlugin>> m_plugins;
};

}  // namespace openhd

#endif  // OPENHD_PLUGIN_MANAGER_H
