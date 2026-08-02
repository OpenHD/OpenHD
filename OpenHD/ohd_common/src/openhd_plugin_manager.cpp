/******************************************************************************
 * OpenHD runtime plugin loader.
 ******************************************************************************/

#include "openhd_plugin_manager.h"

#include <dlfcn.h>

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <sstream>
#include <utility>

#include "openhd_spdlog.h"
#include "openhd_spdlog_include.h"

namespace {

constexpr const char* kDefaultPluginPath = "/usr/local/lib/openhd/plugins";

void host_log(int32_t level, const char* plugin_name, const char* message) {
  const auto logger = openhd::log::create_or_get("plugins");
  const std::string name = plugin_name ? plugin_name : "unknown";
  const std::string text = message ? message : "";
  switch (level) {
    case OPENHD_PLUGIN_LOG_DEBUG:
      logger->debug("[{}] {}", name, text);
      break;
    case OPENHD_PLUGIN_LOG_INFO:
      logger->info("[{}] {}", name, text);
      break;
    case OPENHD_PLUGIN_LOG_WARN:
      logger->warn("[{}] {}", name, text);
      break;
    default:
      logger->error("[{}] {}", name, text);
      break;
  }
}

std::vector<std::string> default_search_paths() {
  const char* configured = std::getenv("OPENHD_PLUGIN_PATH");
  if (!configured || configured[0] == '\0') return {kDefaultPluginPath};

  std::vector<std::string> result;
  std::stringstream stream(configured);
  std::string path;
  while (std::getline(stream, path, ':')) {
    if (!path.empty()) result.push_back(path);
  }
  return result.empty() ? std::vector<std::string>{kDefaultPluginPath}
                        : result;
}

}  // namespace

namespace openhd {

struct PluginManager::LoadedPlugin {
  std::filesystem::path path;
  void* library = nullptr;
  const openhd_plugin_descriptor* descriptor = nullptr;
};

PluginManager& PluginManager::instance() {
  static PluginManager manager;
  return manager;
}

PluginManager::PluginManager() = default;

PluginManager::~PluginManager() {
  // Do not use OpenHD logging from a static destructor: the logger registry may
  // already have been destroyed. main() normally calls shutdown explicitly.
  for (auto it = m_plugins.rbegin(); it != m_plugins.rend(); ++it) {
    const auto& plugin = *it;
    try {
      if (plugin->descriptor->shutdown) plugin->descriptor->shutdown();
    } catch (...) {
    }
    dlclose(plugin->library);
  }
  m_plugins.clear();
}

std::size_t PluginManager::load_plugins(bool is_air) {
  return load_plugins(is_air, default_search_paths());
}

std::size_t PluginManager::load_plugins(
    bool is_air, const std::vector<std::string>& search_paths) {
  const auto logger = openhd::log::create_or_get("plugins");
  const uint32_t role = is_air ? OPENHD_PLUGIN_ROLE_AIR
                               : OPENHD_PLUGIN_ROLE_GROUND;
  const openhd_plugin_host host{sizeof(openhd_plugin_host),
                                OPENHD_PLUGIN_ABI_VERSION, host_log};
  const openhd_plugin_context context{sizeof(openhd_plugin_context), role};

  std::vector<std::filesystem::path> candidates;
  for (const auto& search_path : search_paths) {
    std::error_code ec;
    if (!std::filesystem::is_directory(search_path, ec)) {
      logger->debug("Plugin directory not present: {}", search_path);
      continue;
    }
    for (const auto& entry :
         std::filesystem::directory_iterator(search_path, ec)) {
      if (ec) {
        logger->warn("Cannot scan plugin directory {}: {}", search_path,
                     ec.message());
        break;
      }
      if (entry.is_regular_file(ec) && entry.path().extension() == ".so") {
        candidates.push_back(entry.path());
      }
    }
  }
  std::sort(candidates.begin(), candidates.end());

  for (const auto& path : candidates) {
    void* library = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (!library) {
      logger->error("Cannot load plugin {}: {}", path.string(), dlerror());
      continue;
    }

    dlerror();
    auto get_descriptor = reinterpret_cast<openhd_plugin_get_descriptor_fn>(
        dlsym(library, OPENHD_PLUGIN_ENTRYPOINT));
    const char* symbol_error = dlerror();
    if (symbol_error || !get_descriptor) {
      // Crypto providers share the plugin directory but have a dedicated ABI
      // and are consumed directly by wifibroadcast's packet hot path.
      dlerror();
      if (dlsym(library, "openhd_crypto_get_provider") != nullptr) {
        logger->debug("Leaving crypto provider {} for wifibroadcast",
                      path.string());
        dlclose(library);
        continue;
      }
      logger->error("Ignoring plugin {}: missing {} ({})", path.string(),
                    OPENHD_PLUGIN_ENTRYPOINT,
                    symbol_error ? symbol_error : "unknown error");
      dlclose(library);
      continue;
    }

    const openhd_plugin_descriptor* descriptor = nullptr;
    try {
      descriptor = get_descriptor();
    } catch (...) {
      logger->error("Ignoring plugin {}: descriptor function threw",
                    path.string());
      dlclose(library);
      continue;
    }
    if (!descriptor ||
        descriptor->struct_size < sizeof(openhd_plugin_descriptor) ||
        descriptor->abi_version != OPENHD_PLUGIN_ABI_VERSION ||
        !descriptor->name) {
      logger->error("Ignoring incompatible plugin {}", path.string());
      dlclose(library);
      continue;
    }
    if ((descriptor->supported_roles & role) == 0U) {
      logger->debug("Skipping plugin {} on this unit role", descriptor->name);
      dlclose(library);
      continue;
    }

    int32_t init_result = 0;
    try {
      if (descriptor->init) init_result = descriptor->init(&host, &context);
    } catch (const std::exception& ex) {
      logger->error("Plugin {} initialization threw: {}", descriptor->name,
                    ex.what());
      init_result = -1;
    } catch (...) {
      logger->error("Plugin {} initialization threw", descriptor->name);
      init_result = -1;
    }
    if (init_result != 0) {
      logger->error("Plugin {} initialization failed ({})", descriptor->name,
                    init_result);
      dlclose(library);
      continue;
    }

    auto plugin = std::make_unique<LoadedPlugin>();
    plugin->path = path;
    plugin->library = library;
    plugin->descriptor = descriptor;
    logger->info("Loaded plugin {} v{} from {}", descriptor->name,
                 descriptor->version ? descriptor->version : "unknown",
                 path.string());
    m_plugins.push_back(std::move(plugin));
  }
  return m_plugins.size();
}

void PluginManager::notify_video_bitrate_changed(
    const openhd_plugin_video_bitrate_event& event) noexcept {
  const auto logger = openhd::log::create_or_get("plugins");
  for (const auto& plugin : m_plugins) {
    if (!plugin->descriptor->on_video_bitrate_changed) continue;
    try {
      plugin->descriptor->on_video_bitrate_changed(&event);
    } catch (const std::exception& ex) {
      logger->error("Plugin {} bitrate callback threw: {}",
                    plugin->descriptor->name, ex.what());
    } catch (...) {
      logger->error("Plugin {} bitrate callback threw",
                    plugin->descriptor->name);
    }
  }
}

void PluginManager::shutdown() noexcept {
  if (m_plugins.empty()) return;
  const auto logger = openhd::log::create_or_get("plugins");
  for (auto it = m_plugins.rbegin(); it != m_plugins.rend(); ++it) {
    const auto& plugin = *it;
    try {
      if (plugin->descriptor->shutdown) plugin->descriptor->shutdown();
    } catch (...) {
      logger->error("Plugin {} shutdown callback threw",
                    plugin->descriptor->name);
    }
    dlclose(plugin->library);
  }
  m_plugins.clear();
}

std::size_t PluginManager::loaded_plugin_count() const noexcept {
  return m_plugins.size();
}

}  // namespace openhd
