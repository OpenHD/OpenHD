# 04. Plugin Interface Design

## Interface Objectives
- Define a stable ABI between OpenHD core and plugins.
- Support version negotiation and backward compatibility.
- Allow plugins to declare capabilities, configuration needs, and thread-safety.

## Descriptor Structure
```c++
#pragma once
#include <stdint.h>

#define OPENHD_PLUGIN_API_VERSION 1

#ifdef _WIN32
#define OPENHD_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#else
#define OPENHD_PLUGIN_EXPORT extern "C"
#endif

typedef struct openhd_plugin_descriptor {
    uint32_t api_version;          // Must equal OPENHD_PLUGIN_API_VERSION
    const char *name;              // Human-readable
    const char *version;           // Semantic version string
    const char *description;       // Short summary
    uint32_t capabilities;         // Bitmask for feature flags (e.g., ENCRYPTION=0x1)
    int (*init)(const struct openhd_plugin_context *ctx);
    void (*deinit)(void);
    const struct openhd_encryption_vtable *encryption; // NULL if not provided
} openhd_plugin_descriptor;
```

## Context & VTables
- **Context**: Provided by the core during `init` to supply services (logging, configuration access, telemetry hooks).
  ```c++
  typedef struct openhd_plugin_context {
      uint32_t size; // ABI guard
      void (*log)(int level, const char *component, const char *fmt, ...);
      void *(*allocate)(size_t bytes);
      void (*deallocate)(void *ptr);
      void *user_data; // Optional pointer for core-managed state
  } openhd_plugin_context;
  ```
- **Encryption VTable**: Encapsulates function pointers exposed by the plugin.
  ```c++
  typedef struct openhd_encryption_vtable {
      int (*set_bindphrase)(const char *phrase);
      int (*encrypt_frame)(const uint8_t *in, size_t in_size, uint8_t *out, size_t *out_size);
      int (*decrypt_frame)(const uint8_t *in, size_t in_size, uint8_t *out, size_t *out_size);
      int (*current_status)(struct openhd_encryption_status *status);
  } openhd_encryption_vtable;
  ```

## Capability Flags
- `OPENHD_CAP_ENCRYPTION = 0x0001`
- `OPENHD_CAP_BINDPHRASE = 0x0002`
- Additional flags reserved for telemetry, video encoding, etc.

## Versioning Strategy
- Increment `OPENHD_PLUGIN_API_VERSION` when breaking ABI changes.
- Maintain compatibility by allowing plugins to implement multiple descriptors via optional `openhd_plugin_get_descriptor_vN()` exports (future).
- Provide helper `openhd::PluginVersion` struct to compare plugin semantic versions.

## Plugin Entry Point
- Each plugin exports exactly one symbol:
  ```c++
  OPENHD_PLUGIN_EXPORT const openhd_plugin_descriptor* openhd_plugin_get_descriptor();
  ```
- Manager validates `descriptor->api_version`.
- Manager stores pointer to descriptor for the lifetime of the plugin (must remain valid).

## Core Consumption Pattern
```c++
const openhd_plugin_descriptor *desc = load_descriptor(path);
if (desc->capabilities & OPENHD_CAP_ENCRYPTION) {
    active_encryption_vtable = desc->encryption;
    desc->init(&core_context);
}
```

## Safety Notes
- All structs padded to avoid alignment issues; document requirement for `sizeof(openhd_plugin_context)`.
- Use `extern "C"` to avoid name mangling.
- Document thread-safety expectations explicitly in descriptor description field.

## Automation Prompt
```
You are assisting with "04. Plugin Interface Design" for the OpenHD plugin system manual. Produce header-style code blocks for descriptor, context, and encryption vtable definitions, list capability flags, and explain versioning and entry point conventions.
```
