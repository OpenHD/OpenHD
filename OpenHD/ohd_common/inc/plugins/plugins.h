#ifndef OPENHD_PLUGINS_H
#define OPENHD_PLUGINS_H

#include <cstddef>
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

// Defines the major version of the OpenHD plugin ABI.
#define OPENHD_PLUGIN_ABI_MAJOR 1
// Defines the minor version of the OpenHD plugin ABI.
#define OPENHD_PLUGIN_ABI_MINOR 0
// Composed ABI version used to verify compatibility between host and plugin.
#define OPENHD_PLUGIN_API_VERSION ((OPENHD_PLUGIN_ABI_MAJOR << 16) | OPENHD_PLUGIN_ABI_MINOR)

// Enumerates known plugin categories recognized by OpenHD.
enum class openhd_plugin_type {
    UNKNOWN = 0,   // Plugin type is not specified.
    ENCRYPTION,    // Plugin implements encryption functionality.
    MAX            // Sentinel value marking the end of valid types.
};

// Contains metadata describing a plugin's capabilities and ABI compatibility.
struct openhd_plugin_info {
    uint32_t abi_version;        // ABI version expected by the plugin.
    const char *name;            // Human-readable name of the plugin.
    const char *description;     // Brief description of the plugin's functionality.
    openhd_plugin_type type;     // Category of the plugin.
    uint32_t capabilities;       // Bitmask describing plugin-specific capabilities.
};

// Forward declaration of the opaque plugin context provided by the implementation.
struct openhd_plugin_context;

// Forward declaration for encryption-specific dispatch table exposed by plugins.
struct openhd_encryption_vtable;

// Retrieves plugin metadata for compatibility and discovery purposes.
typedef const struct openhd_plugin_info *(*openhd_plugin_get_info_fn)(void);

// Initializes a plugin instance using the provided host context.
typedef struct openhd_plugin_context *(*openhd_plugin_init_fn)(void *host_context);

// Shuts down a previously initialized plugin instance and releases resources.
typedef void (*openhd_plugin_shutdown_fn)(struct openhd_plugin_context *context);

// Generic dispatch table shared by all plugins. Individual plugin categories may extend the
// payload with additional type-specific data (for example encryption related entry points).
struct openhd_plugin_vtable {
    openhd_plugin_shutdown_fn shutdown;  // Mandatory shutdown callback for the plugin instance.
    void *payload;                       // Optional pointer to category specific dispatch tables.
};

// Optional callback exposed by plugins that want to provide a richer dispatch table. The function
// is expected to populate the supplied vtable structure and return true on success.
typedef bool (*openhd_plugin_query_vtable_fn)(struct openhd_plugin_context *context,
                                              struct openhd_plugin_vtable *out_vtable);

// Aggregates exported entry points and optional payloads for type-specific data.
struct openhd_plugin_exports {
    openhd_plugin_get_info_fn get_info;   // Retrieves static plugin information.
    openhd_plugin_init_fn init;           // Initializes the plugin and returns its context.
    openhd_plugin_shutdown_fn shutdown;   // Releases resources associated with the plugin context.
    void *payload;                        // Optional pointer to additional type-specific exports.
};

#ifdef __cplusplus
}
#endif

#endif  // OPENHD_PLUGINS_H
