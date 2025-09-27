# 02. Plugin Architecture

## Architectural Goals
- Modularize optional subsystems via dynamically loaded plugins.
- Maintain a minimal core binary with extension points for features like encryption.
- Provide deterministic lifecycle management (init → runtime calls → deinit).

## Key Components
- **Plugin Manager** (`plugin_manager.cpp/.h`)
  - Discovers plugin binaries (configurable directories, e.g., `/usr/lib/openhd/plugins`).
  - Loads libraries using platform-appropriate APIs.
  - Registers and tracks plugin instances.
- **Plugin Interface Header** (`include/openhd/plugin.h`)
  - Defines the ABI contract (function pointers, struct layout, version identifiers).
  - Provides macros for exporting symbols (`OPENHD_PLUGIN_EXPORT`).
- **Core Integration Layer**
  - Hooks into existing OpenHD modules (e.g., telemetry, encryption handlers).
  - Delegates functionality to loaded plugins when available.
- **Plugin Implementations**
  - Standalone shared libraries packaged separately.
  - Follow naming convention `libopenhd_<feature>_plugin.so` or `openhd_<feature>.dll`.

## Data Flow Overview
1. Configuration instructs the manager which plugins to load (e.g., `openhd.conf`).
2. Manager scans directories, filters by naming convention, and loads each library.
3. Manager resolves `openhd_plugin_get_descriptor()` to obtain metadata & function table.
4. Manager validates API version, registers capabilities, and calls `init` with context.
5. Core subsystems invoke plugin functions through the manager.
6. On shutdown or reload, manager calls `deinit` and unloads the library.

## Lifecycle States
- **Discovered** → file path identified, pending validation.
- **Loaded** → shared object opened, descriptor retrieved.
- **Initialized** → plugin `init` executed successfully.
- **Active** → plugin functions callable.
- **Failed** → plugin rejected; fallback path triggered.

## Threading & Concurrency Considerations
- Access to plugin registry guarded via mutex or reader-writer lock.
- Plugin callbacks executed on worker thread(s) determined by caller context.
- Document requirement: plugins must be thread-safe or note thread affinity in metadata.

## Error Handling Strategy
- Centralize error codes/enums for plugin loading failures (file missing, symbol missing, version mismatch).
- Log failures with actionable diagnostics (`OPENHD_LOG_ERROR`).
- Provide fallback path (internal stub) when plugin fails.

## Configuration Hooks
- Extend existing configuration to include plugin directories and load order.
- Support environment variable overrides (`OPENHD_PLUGIN_PATH`).
- Allow runtime reload trigger (optional) via command or signal.

## Automation Prompt
```
You are assisting with "02. Plugin Architecture" for the OpenHD plugin system manual. Describe the core components (manager, interface, implementations), lifecycle, data flow, and error handling using bullet lists and numbered steps. Include configuration and concurrency considerations. Focus on guidance for implementing architecture scaffolding without writing final code.
```
