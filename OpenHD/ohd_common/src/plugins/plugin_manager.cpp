#include "plugins/plugin_manager.h"

#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <filesystem>
#include <string>

struct openhd_plugin_symbol_table {
  openhd_plugin_get_info_fn get_info;
  openhd_plugin_init_fn init;
  openhd_plugin_shutdown_fn shutdown;
  openhd_plugin_query_vtable_fn query_vtable;
};

namespace {

void log_loader_error(const char *message, const char *detail = nullptr) {
  if (!message) {
    return;
  }
  if (detail) {
    std::fprintf(stderr, "[OpenHD][plugins] %s: %s\n", message, detail);
  } else {
    std::fprintf(stderr, "[OpenHD][plugins] %s\n", message);
  }
}

bool has_shared_library_extension(const std::filesystem::path &path) {
  const auto filename = path.filename().string();
#if defined(_WIN32)
  const std::string suffix = ".dll";
  if (filename.size() < suffix.size()) {
    return false;
  }
  const auto tail = filename.substr(filename.size() - suffix.size());
  std::string lower_tail;
  lower_tail.resize(tail.size());
  std::transform(
      tail.begin(), tail.end(), lower_tail.begin(),
      [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return lower_tail == suffix;
#elif defined(__APPLE__)
  static const char *const kSuffixes[] = {".dylib", ".so"};
  for (const auto *suffix : kSuffixes) {
    const auto len = std::strlen(suffix);
    if (filename.size() >= len &&
        filename.compare(filename.size() - len, len, suffix) == 0) {
      return true;
    }
  }
  return false;
#else
  static const char *const kSuffixes[] = {".so"};
  for (const auto *suffix : kSuffixes) {
    const auto len = std::strlen(suffix);
    if (filename.size() >= len &&
        filename.compare(filename.size() - len, len, suffix) == 0) {
      return true;
    }
  }
  const auto so_pos = filename.find(".so.");
  return so_pos != std::string::npos;
#endif
}

}  // namespace

static void openhd_loaded_plugin_reset(struct openhd_loaded_plugin *plugin) {
  if (!plugin) {
    return;
  }

  plugin->dl_handle = NULL;
  plugin->instance = NULL;
  memset(&plugin->info, 0, sizeof(plugin->info));
  memset(&plugin->vtable, 0, sizeof(plugin->vtable));
  plugin->initialized = false;
  plugin->load_failed = false;
}

static bool openhd_plugin_manager_ensure_plugin_capacity(
    struct openhd_plugin_manager *mgr) {
  if (mgr->plugin_count < mgr->plugin_capacity) {
    return true;
  }

  size_t new_capacity =
      mgr->plugin_capacity == 0 ? 4 : mgr->plugin_capacity * 2;
  struct openhd_loaded_plugin *new_plugins = NULL;

  if (mgr->plugin_capacity == 0) {
    new_plugins = (struct openhd_loaded_plugin *)calloc(new_capacity,
                                                        sizeof(*new_plugins));
  } else {
    new_plugins = (struct openhd_loaded_plugin *)realloc(
        mgr->plugins, new_capacity * sizeof(*new_plugins));
  }
  if (!new_plugins) {
    log_loader_error("Failed to allocate memory for plugin table");
    return false;
  }

  for (size_t i = mgr->plugin_capacity; i < new_capacity; ++i) {
    openhd_loaded_plugin_reset(&new_plugins[i]);
  }

  mgr->plugins = new_plugins;
  mgr->plugin_capacity = new_capacity;
  return true;
}

static bool openhd_plugin_manager_ensure_path_capacity(
    struct openhd_plugin_manager *mgr) {
  if (mgr->search_path_count < mgr->search_path_capacity) {
    return true;
  }

  size_t new_capacity =
      mgr->search_path_capacity == 0 ? 4 : mgr->search_path_capacity * 2;
  char **new_paths = NULL;

  if (mgr->search_path_capacity == 0) {
    new_paths = (char **)calloc(new_capacity, sizeof(*new_paths));
  } else {
    new_paths =
        (char **)realloc(mgr->search_paths, new_capacity * sizeof(*new_paths));
  }
  if (!new_paths) {
    log_loader_error("Failed to allocate memory for plugin search paths");
    return false;
  }

  for (size_t i = mgr->search_path_capacity; i < new_capacity; ++i) {
    new_paths[i] = NULL;
  }

  mgr->search_paths = new_paths;
  mgr->search_path_capacity = new_capacity;
  return true;
}

static void openhd_plugin_manager_release_plugin(
    struct openhd_loaded_plugin *plugin) {
  if (!plugin) {
    return;
  }

  if (plugin->initialized && plugin->vtable.shutdown && plugin->instance) {
    plugin->vtable.shutdown(plugin->instance);
  }

  if (plugin->dl_handle) {
    dlclose(plugin->dl_handle);
  }

  openhd_loaded_plugin_reset(plugin);
}

static bool openhd_plugin_manager_load_symbols(
    void *handle, struct openhd_plugin_symbol_table *out_symbols) {
  if (!handle || !out_symbols) {
    return false;
  }

  memset(out_symbols, 0, sizeof(*out_symbols));

  out_symbols->get_info =
      (openhd_plugin_get_info_fn)dlsym(handle, "openhd_plugin_get_info");
  out_symbols->init =
      (openhd_plugin_init_fn)dlsym(handle, "openhd_plugin_init");
  out_symbols->shutdown =
      (openhd_plugin_shutdown_fn)dlsym(handle, "openhd_plugin_shutdown");
  out_symbols->query_vtable = (openhd_plugin_query_vtable_fn)dlsym(
      handle, "openhd_plugin_query_vtable");

  if (!out_symbols->get_info || !out_symbols->init || !out_symbols->shutdown) {
    log_loader_error("Plugin is missing required entry points");
    return false;
  }

  return true;
}

void openhd_plugin_manager_init(struct openhd_plugin_manager *mgr) {
  if (!mgr) {
    return;
  }

  memset(mgr, 0, sizeof(*mgr));
}

void openhd_plugin_manager_shutdown(struct openhd_plugin_manager *mgr) {
  if (!mgr) {
    return;
  }

  for (size_t i = 0; i < mgr->plugin_count; ++i) {
    openhd_plugin_manager_release_plugin(&mgr->plugins[i]);
  }

  free(mgr->plugins);
  mgr->plugins = NULL;
  mgr->plugin_count = 0;
  mgr->plugin_capacity = 0;

  for (size_t i = 0; i < mgr->search_path_count; ++i) {
    free(mgr->search_paths[i]);
  }

  free(mgr->search_paths);
  mgr->search_paths = NULL;
  mgr->search_path_count = 0;
  mgr->search_path_capacity = 0;
}

bool openhd_plugin_manager_add_search_path(struct openhd_plugin_manager *mgr,
                                           const char *path) {
  if (!mgr || !path || path[0] == '\0') {
    return false;
  }

  for (size_t i = 0; i < mgr->search_path_count; ++i) {
    if (mgr->search_paths[i] && std::strcmp(mgr->search_paths[i], path) == 0) {
      return true;
    }
  }

  if (!openhd_plugin_manager_ensure_path_capacity(mgr)) {
    return false;
  }

  char *copy = strdup(path);
  if (!copy) {
    // TODO: Replace with OpenHD logging when available.
    return false;
  }

  mgr->search_paths[mgr->search_path_count++] = copy;
  return true;
}

bool openhd_plugin_manager_load(struct openhd_plugin_manager *mgr,
                                const char *file_path, void *host_context) {
  if (!mgr || !file_path || file_path[0] == '\0') {
    return false;
  }

  void *handle = dlopen(file_path, RTLD_NOW);
  if (!handle) {
    const char *error = dlerror();
    log_loader_error("dlopen failed", error ? error : file_path);
    return false;
  }

  struct openhd_plugin_symbol_table symbols;
  if (!openhd_plugin_manager_load_symbols(handle, &symbols)) {
    dlclose(handle);
    return false;
  }

  const struct openhd_plugin_info *info = symbols.get_info();
  if (!info) {
    log_loader_error("Plugin did not return metadata", file_path);
    dlclose(handle);
    return false;
  }

  if (info->abi_version != OPENHD_PLUGIN_API_VERSION) {
    log_loader_error("Plugin ABI version mismatch", file_path);
    dlclose(handle);
    return false;
  }

  struct openhd_plugin_context *instance = symbols.init(host_context);
  if (!instance) {
    log_loader_error("Plugin failed to initialise", file_path);
    dlclose(handle);
    return false;
  }

  struct openhd_plugin_vtable vtable;
  memset(&vtable, 0, sizeof(vtable));

  if (symbols.query_vtable) {
    if (!symbols.query_vtable(instance, &vtable)) {
      log_loader_error("Plugin rejected vtable query", file_path);
      symbols.shutdown(instance);
      dlclose(handle);
      return false;
    }
  }

  if (!vtable.shutdown) {
    vtable.shutdown = symbols.shutdown;
  }

  if (!openhd_plugin_manager_ensure_plugin_capacity(mgr)) {
    symbols.shutdown(instance);
    dlclose(handle);
    return false;
  }

  struct openhd_loaded_plugin *slot = &mgr->plugins[mgr->plugin_count];
  openhd_loaded_plugin_reset(slot);
  slot->dl_handle = handle;
  slot->instance = instance;
  memcpy(&slot->info, info, sizeof(slot->info));
  memcpy(&slot->vtable, &vtable, sizeof(slot->vtable));
  slot->initialized = true;
  slot->load_failed = false;

  mgr->plugin_count += 1;
  return true;
}

bool openhd_plugin_manager_load_all(struct openhd_plugin_manager *mgr,
                                    void *host_context) {
  if (!mgr) {
    return false;
  }

  bool loaded_any = false;
  for (size_t i = 0; i < mgr->search_path_count; ++i) {
    const char *raw_path = mgr->search_paths[i];
    if (!raw_path || raw_path[0] == '\0') {
      continue;
    }

    const std::filesystem::path directory(raw_path);
    std::error_code ec;
    if (!std::filesystem::exists(directory, ec) ||
        !std::filesystem::is_directory(directory, ec)) {
      continue;
    }

    for (std::filesystem::directory_iterator it(directory, ec);
         !ec && it != std::filesystem::directory_iterator(); ++it) {
      const std::filesystem::path &entry_path = it->path();
      std::error_code type_ec;
      if (!it->is_regular_file(type_ec)) {
        continue;
      }

      if (!has_shared_library_extension(entry_path)) {
        continue;
      }

      const std::string candidate = entry_path.string();
      if (openhd_plugin_manager_load(mgr, candidate.c_str(), host_context)) {
        loaded_any = true;
      }
    }

    if (ec) {
      log_loader_error("Failed to iterate plugin directory", raw_path);
    }
  }

  return loaded_any;
}

void openhd_plugin_manager_foreach(struct openhd_plugin_manager *mgr,
                                   openhd_plugin_iterate_fn fn,
                                   void *userdata) {
  if (!mgr || !fn) {
    return;
  }

  for (size_t i = 0; i < mgr->plugin_count; ++i) {
    if (!fn(&mgr->plugins[i], userdata)) {
      break;
    }
  }
}

struct openhd_loaded_plugin *openhd_plugin_manager_get_by_type(
    struct openhd_plugin_manager *mgr, enum openhd_plugin_type type) {
  if (!mgr) {
    return NULL;
  }

  for (size_t i = 0; i < mgr->plugin_count; ++i) {
    if (mgr->plugins[i].initialized && mgr->plugins[i].info.type == type) {
      return &mgr->plugins[i];
    }
  }

  return NULL;
}
