#include "plugins/plugin_manager.h"

#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

struct openhd_plugin_symbol_table {
  openhd_plugin_get_info_fn get_info;
  openhd_plugin_init_fn init;
  openhd_plugin_shutdown_fn shutdown;
  openhd_plugin_query_vtable_fn query_vtable;
};

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
    // TODO: Replace with OpenHD logging when available.
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
    // TODO: Replace with OpenHD logging when available.
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
    // TODO: Replace with OpenHD logging when available.
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
  mgr->encryption_cache_index = 0;
  mgr->encryption_cache_valid = false;
  mgr->encryption_vtable_cache = NULL;
}

bool openhd_plugin_manager_add_search_path(struct openhd_plugin_manager *mgr,
                                           const char *path) {
  if (!mgr || !path || path[0] == '\0') {
    return false;
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
    // TODO: Replace with OpenHD logging when available.
    return false;
  }

  struct openhd_plugin_symbol_table symbols;
  if (!openhd_plugin_manager_load_symbols(handle, &symbols)) {
    dlclose(handle);
    return false;
  }

  const struct openhd_plugin_info *info = symbols.get_info();
  if (!info) {
    // TODO: Replace with OpenHD logging when available.
    dlclose(handle);
    return false;
  }

  if (info->abi_version != OPENHD_PLUGIN_API_VERSION) {
    // TODO: Replace with OpenHD logging when available.
    dlclose(handle);
    return false;
  }

  struct openhd_plugin_context *instance = symbols.init(host_context);
  if (!instance) {
    // TODO: Replace with OpenHD logging when available.
    dlclose(handle);
    return false;
  }

  struct openhd_plugin_vtable vtable;
  memset(&vtable, 0, sizeof(vtable));

  if (symbols.query_vtable) {
    if (!symbols.query_vtable(instance, &vtable)) {
      // TODO: Replace with OpenHD logging when available.
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

  size_t slot_index = mgr->plugin_count;
  mgr->plugin_count += 1;
  if (slot->info.type == OPENHD_PLUGIN_TYPE_ENCRYPTION) {
    mgr->encryption_cache_valid = true;
    mgr->encryption_cache_index = slot_index;
    mgr->encryption_vtable_cache = &mgr->plugins[slot_index].vtable;
  }
  return true;
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

const struct openhd_plugin_vtable *openhd_plugin_manager_get_encryption_vtable(
    struct openhd_plugin_manager *mgr) {
  if (!mgr) {
    return NULL;
  }

  if (mgr->encryption_cache_valid) {
    if (mgr->encryption_cache_index < mgr->plugin_count) {
      struct openhd_loaded_plugin *plugin =
          &mgr->plugins[mgr->encryption_cache_index];
      if (plugin->initialized &&
          plugin->info.type == OPENHD_PLUGIN_TYPE_ENCRYPTION) {
        mgr->encryption_vtable_cache = &plugin->vtable;
        return mgr->encryption_vtable_cache;
      }
    }
    mgr->encryption_cache_valid = false;
    mgr->encryption_vtable_cache = NULL;
  }

  struct openhd_loaded_plugin *plugin =
      openhd_plugin_manager_get_by_type(mgr, OPENHD_PLUGIN_TYPE_ENCRYPTION);
  if (!plugin) {
    return NULL;
  }

  mgr->encryption_cache_valid = true;
  mgr->encryption_cache_index = (size_t)(plugin - mgr->plugins);
  mgr->encryption_vtable_cache = &mgr->plugins[mgr->encryption_cache_index].vtable;
  return mgr->encryption_vtable_cache;
}
