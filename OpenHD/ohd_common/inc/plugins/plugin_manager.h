#ifndef OPENHD_PLUGIN_MANAGER_H
#define OPENHD_PLUGIN_MANAGER_H

#include <stdbool.h>
#include <stddef.h>

#include "plugins/plugins.h"

#ifdef __cplusplus
extern "C" {
#endif

struct openhd_loaded_plugin {
    void *dl_handle;
    struct openhd_plugin *instance;
    struct openhd_plugin_info info;
    struct openhd_plugin_vtable vtable;
    bool initialized;
    bool load_failed;
};

struct openhd_plugin_manager {
    struct openhd_loaded_plugin *plugins;
    size_t plugin_count;
    size_t plugin_capacity;
    char **search_paths;
    size_t search_path_count;
    size_t search_path_capacity;
};

typedef bool (*openhd_plugin_iterate_fn)(struct openhd_loaded_plugin *plugin, void *userdata);

void openhd_plugin_manager_init(struct openhd_plugin_manager *mgr);
void openhd_plugin_manager_shutdown(struct openhd_plugin_manager *mgr);

bool openhd_plugin_manager_add_search_path(struct openhd_plugin_manager *mgr, const char *path);

bool openhd_plugin_manager_load(struct openhd_plugin_manager *mgr, const char *file_path, void *host_context);

void openhd_plugin_manager_foreach(struct openhd_plugin_manager *mgr, openhd_plugin_iterate_fn fn, void *userdata);

struct openhd_loaded_plugin *openhd_plugin_manager_get_by_type(struct openhd_plugin_manager *mgr,
                                                               enum openhd_plugin_type type);

#ifdef __cplusplus
}
#endif

#endif  // OPENHD_PLUGIN_MANAGER_H
