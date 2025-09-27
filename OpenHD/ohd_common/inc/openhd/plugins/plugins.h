#ifndef OPENHD_PLUGINS_PLUGINS_H
#define OPENHD_PLUGINS_PLUGINS_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OPENHD_PLUGIN_ABI_MAJOR 1
#define OPENHD_PLUGIN_ABI_MINOR 0
#define OPENHD_PLUGIN_API_VERSION ((OPENHD_PLUGIN_ABI_MAJOR << 16) | OPENHD_PLUGIN_ABI_MINOR)

enum openhd_plugin_type {
    OPENHD_PLUGIN_TYPE_UNKNOWN = 0,
    OPENHD_PLUGIN_TYPE_ENCRYPTION,
    OPENHD_PLUGIN_TYPE_MAX
};

struct openhd_plugin_info {
    uint32_t abi_version;
    const char *name;
    const char *description;
    enum openhd_plugin_type type;
    uint32_t capabilities;
};

struct openhd_plugin_context;

struct openhd_plugin {
    struct openhd_plugin_context *context;
};

struct openhd_plugin_vtable;

typedef const struct openhd_plugin_info *(*openhd_plugin_get_info_fn)(void);
typedef struct openhd_plugin *(*openhd_plugin_init_fn)(void *host_context);
typedef void (*openhd_plugin_shutdown_fn)(struct openhd_plugin *plugin);

typedef bool (*openhd_plugin_query_vtable_fn)(struct openhd_plugin *plugin,
                                               struct openhd_plugin_vtable *out_vtable);

struct openhd_plugin_vtable {
    openhd_plugin_shutdown_fn shutdown;
    void *userdata;
};

#ifdef __cplusplus
}
#endif

#endif  // OPENHD_PLUGINS_PLUGINS_H
