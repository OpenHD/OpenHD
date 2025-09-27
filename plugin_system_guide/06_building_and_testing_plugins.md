# 06. Building & Testing Plugins

## Build Configuration
- Add cache options in top-level `CMakeLists.txt`:
  ```cmake
  option(OPENHD_ENABLE_PLUGINS "Enable plugin subsystem" ON)
  option(OPENHD_BUILD_ENCRYPTION_PLUGIN "Build bundled encryption plugin" ON)
  set(OPENHD_PLUGIN_OUTPUT_DIR "${CMAKE_BINARY_DIR}/plugins" CACHE PATH "Runtime plugin directory")
  ```
- Wrap plugin-specific logic with `if (OPENHD_ENABLE_PLUGINS)` guards.
- Expose output directory to runtime via configuration file (`openhd.conf`).

## Full Build Sequence
1. Configure project with plugins enabled:
   ```bash
   cmake -S . -B build -DOPENHD_ENABLE_PLUGINS=ON -DOPENHD_BUILD_ENCRYPTION_PLUGIN=ON \
         -DOPENHD_PLUGIN_OUTPUT_DIR=$PWD/build/plugins
   ```
2. Build targets:
   ```bash
   cmake --build build --target openhd openhd_encryption_plugin
   ```
3. Install artifacts into staging directory:
   ```bash
   cmake --install build --prefix $PWD/stage
   ```
4. Verify plugin placement:
   ```bash
   ls -R build/plugins
   ls stage/lib/openhd/plugins
   ```

## Unit & Integration Tests
- Extend existing test suites with plugin-aware cases:
  - Create `tests/plugin_manager_tests.cpp` verifying load success/failure paths.
  - Mock plugin descriptors for unit tests without requiring real `.so`.
- Example `ctest` invocation:
  ```bash
  ctest --test-dir build --output-on-failure -R plugin
  ```
- Add runtime integration test script `scripts/test_encryption_plugin.sh`:
  ```bash
  #!/usr/bin/env bash
  set -euo pipefail
  export OPENHD_PLUGIN_PATH="$PWD/build/plugins"
  build/openhd --config configs/encryption_plugin.conf --dry-run
  ```

## Continuous Integration Updates
- Update `.github/workflows/build.yml` (or equivalent CI) to:
  - Add matrix entry `OPENHD_ENABLE_PLUGINS=ON`.
  - Upload plugin `.so` as artifact for downstream packaging.
  - Run smoke tests with plugin path configured.
- Add caching for build dependencies to reduce CI time.

## Local Debugging Tips
- Set `OPENHD_LOG_LEVEL=debug` to trace plugin load/unload events.
- Use `ldd build/plugins/libopenhd_encryption.so` to confirm dependency resolution.
- Leverage `strace -e openat` (Linux) to ensure plugin search paths are correct.

## Packaging Considerations
- Document plugin directory layout in packaging scripts (`package.sh`).
- Ensure Debian/RPM spec files install plugin to `/usr/lib/openhd/plugins`.
- Provide `postinst` script to create plugin directory with correct permissions.

## Automation Prompt
```
You are assisting with "06. Building & Testing Plugins" for the OpenHD plugin system manual. Produce CMake configuration snippets, shell commands for building/installing/testing, CI checklist updates, and debugging tips. Use bullet lists and fenced code blocks.
```
