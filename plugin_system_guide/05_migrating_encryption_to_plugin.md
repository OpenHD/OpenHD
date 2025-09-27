# 05. Migrating Encryption to a Plugin

## Objective
- Extract all encryption/bindphrase logic from OpenHD core into `plugins/encryption/` shared library.
- Maintain functional parity with existing features while allowing optional plugin deployment.

## Phase 1: Inventory Existing Logic
1. Search for relevant keywords:
   ```bash
   rg "bindphrase" -n OpenHD/
   rg "encrypt" -n OpenHD/
   rg "decrypt" -n OpenHD/
   rg "libsodium" -n
   ```
2. Catalog:
   - Source files containing encryption functions (e.g., `OpenHD/src/encryption.cpp`).
   - Configuration interfaces (CLI flags, config files).
   - Data structures or enums referencing bindphrase.
   - Unit/integration tests covering encryption.
3. Document dependencies (e.g., libsodium, OpenSSL) and confirm they can be linked from plugin.

## Phase 2: Define Core ↔ Plugin Boundary
- Map each function to plugin vtable entry (encrypt/decrypt/set_bindphrase/status).
- Identify any global state to move into plugin-owned static or context-managed storage.
- Decide which configuration flows remain in core (e.g., loading bindphrase string) and which move into plugin.

## Phase 3: Create Plugin Skeleton
1. Create directory structure:
   ```bash
   mkdir -p plugins/encryption/include
   mkdir -p plugins/encryption/src
   ```
2. Add `plugins/encryption/CMakeLists.txt`:
   ```cmake
   add_library(openhd_encryption_plugin SHARED
       src/encryption_plugin.cpp
       src/bindphrase_manager.cpp
   )
   target_include_directories(openhd_encryption_plugin
       PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
       PUBLIC ${CMAKE_SOURCE_DIR}/include
   )
   target_link_libraries(openhd_encryption_plugin
       PRIVATE sodium
   )
   set_target_properties(openhd_encryption_plugin PROPERTIES
       OUTPUT_NAME "openhd_encryption"
       LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
   )
   ```
3. Add to root `CMakeLists.txt`:
   ```cmake
   add_subdirectory(plugins/encryption)
   install(TARGETS openhd_encryption_plugin
       LIBRARY DESTINATION lib/openhd/plugins
       RUNTIME DESTINATION lib/openhd/plugins
   )
   install(FILES plugins/encryption/plugin.json DESTINATION share/openhd/plugins)
   ```

## Phase 4: Move Implementation
1. Copy encryption headers into `plugins/encryption/include/` and adjust includes to reference `openhd/plugin.h`.
2. Implement descriptor and vtable in `src/encryption_plugin.cpp`:
   ```c++
   #include "openhd/plugin.h"
   #include "bindphrase_manager.h"

   static openhd_encryption_vtable g_vtable {
       &set_bindphrase_impl,
       &encrypt_frame_impl,
       &decrypt_frame_impl,
       &current_status_impl
   };

   static openhd_plugin_descriptor g_descriptor {
       OPENHD_PLUGIN_API_VERSION,
       "OpenHD Encryption",
       "1.0.0",
       "Provides encryption & bindphrase handling",
       OPENHD_CAP_ENCRYPTION | OPENHD_CAP_BINDPHRASE,
       &plugin_init,
       &plugin_deinit,
       &g_vtable
   };

   OPENHD_PLUGIN_EXPORT const openhd_plugin_descriptor* openhd_plugin_get_descriptor() {
       return &g_descriptor;
   }
   ```
3. Port logic from original files into plugin sources, updating namespaces and removing direct dependencies on OpenHD globals.
4. Provide `plugin_init` to capture context pointers and configure logging.

## Phase 5: Update Core to Use Plugin
1. Add plugin manager calls in initialization path (e.g., `OpenHD/src/main.cpp`):
   ```c++
   plugin_manager.load_all();
   auto encryption = plugin_manager.get_encryption();
   if (encryption) {
       encryption->set_bindphrase(config.bindphrase.c_str());
   } else {
       OPENHD_LOG_WARN("Encryption plugin unavailable; running without encryption");
   }
   ```
2. Replace direct encryption calls with `encryption->encrypt_frame(...)` wrappers.
3. Remove old encryption sources from `target_sources(openhd ...)`.
4. Keep lightweight compatibility stubs to log warnings when plugin missing.

## Phase 6: Configuration & Packaging
- Update configuration docs to mention plugin requirement and path.
- Ensure runtime packaging copies `.so` plus optional `plugin.json` metadata.
- Provide fallback config flag `enable_encryption_plugin=false` for builds without plugin.

## Phase 7: Verification
1. Build plugin and core:
   ```bash
   cmake -S . -B build -DENABLE_ENCRYPTION_PLUGIN=ON
   cmake --build build --target openhd openhd_encryption_plugin
   ```
2. Run existing tests to confirm no regressions:
   ```bash
   ctest --test-dir build
   ```
3. Execute runtime smoke test:
   ```bash
   build/openhd --config openhd.conf --log-level debug
   ```
   - Observe logs confirming plugin load and bindphrase application.
4. Test fallback by removing plugin from search path and verifying warnings only.

## Automation Prompt
```
You are assisting with "05. Migrating Encryption to a Plugin" for the OpenHD plugin system manual. Produce step-by-step shell commands, CMake snippets, and C++ templates to move encryption/bindphrase logic into a new plugin directory. Ensure you cover inventory, skeleton creation, core updates, packaging, and verification.
```
