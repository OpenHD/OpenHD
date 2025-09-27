# 03. Dynamic Loading Mechanics

## Platform Abstractions
- Use `#ifdef _WIN32` blocks to differentiate between Windows and POSIX.
- Wrap platform APIs in `openhd::DynamicLibrary` helper class with methods:
  - `bool open(const std::string &path);`
  - `void close();`
  - `void* resolve(const char *symbol);`
  - `std::string last_error() const;`
- POSIX implementation leverages `dlopen`, `dlclose`, `dlsym`, `dlerror`.
- Windows implementation uses `LoadLibraryA`, `FreeLibrary`, `GetProcAddress`, `GetLastError`.

## Step-by-Step Implementation (POSIX-first)
1. **Create abstraction header** `include/openhd/dylib.h`:
   ```c++
   #pragma once
   #include <string>

   namespace openhd {
   class DynamicLibrary {
   public:
       DynamicLibrary() = default;
       ~DynamicLibrary();

       bool open(const std::string &path);
       void close();
       void* resolve(const char *symbol) const;
       std::string last_error() const;

   private:
       void *handle_ {nullptr};
   };
   }
   ```
2. **Add POSIX source** `src/dylib_posix.cpp`:
   ```c++
   #include "openhd/dylib.h"
   #include <dlfcn.h>

   namespace openhd {
   DynamicLibrary::~DynamicLibrary() { close(); }

   bool DynamicLibrary::open(const std::string &path) {
       handle_ = dlopen(path.c_str(), RTLD_NOW);
       return handle_ != nullptr;
   }

   void DynamicLibrary::close() {
       if (handle_) {
           dlclose(handle_);
           handle_ = nullptr;
       }
   }

   void* DynamicLibrary::resolve(const char *symbol) const {
       return handle_ ? dlsym(handle_, symbol) : nullptr;
   }

   std::string DynamicLibrary::last_error() const {
       const char *err = dlerror();
       return err ? std::string(err) : std::string();
   }
   }
   ```
3. **Add Windows source** `src/dylib_win.cpp` (compiled only on `_WIN32`):
   ```c++
   #include "openhd/dylib.h"
   #include <windows.h>

   namespace openhd {
   DynamicLibrary::~DynamicLibrary() { close(); }

   bool DynamicLibrary::open(const std::string &path) {
       handle_ = LoadLibraryA(path.c_str());
       return handle_ != nullptr;
   }

   void DynamicLibrary::close() {
       if (handle_) {
           FreeLibrary(static_cast<HMODULE>(handle_));
           handle_ = nullptr;
       }
   }

   void* DynamicLibrary::resolve(const char *symbol) const {
       return handle_ ? reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(handle_), symbol)) : nullptr;
   }

   std::string DynamicLibrary::last_error() const {
       DWORD error = GetLastError();
       if (!error) return {};
       LPSTR buffer = nullptr;
       size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                    nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                    reinterpret_cast<LPSTR>(&buffer), 0, nullptr);
       std::string message(buffer, size);
       LocalFree(buffer);
       return message;
   }
   }
   ```

## CMake Integration
- Update top-level `CMakeLists.txt`:
  ```cmake
  target_sources(openhd PRIVATE
      src/dylib_posix.cpp
  )
  if (WIN32)
      target_sources(openhd PRIVATE src/dylib_win.cpp)
      target_link_libraries(openhd PRIVATE ws2_32)
  else()
      target_link_libraries(openhd PRIVATE dl)
  endif()
  ```
- Ensure include directory (`include/`) is exposed via `target_include_directories`.

## Runtime Loading Example
```c++
openhd::DynamicLibrary lib;
if (!lib.open(candidate_path)) {
    OPENHD_LOG_ERROR("Failed to open %s: %s", candidate_path.c_str(), lib.last_error().c_str());
    return PluginLoadResult::FileOpenFailed;
}
auto symbol = reinterpret_cast<openhd_plugin_descriptor*(*)()>(lib.resolve("openhd_plugin_get_descriptor"));
if (!symbol) {
    OPENHD_LOG_ERROR("Missing descriptor symbol in %s", candidate_path.c_str());
    lib.close();
    return PluginLoadResult::MissingSymbol;
}
auto *descriptor = symbol();
```

## Diagnostics & Logging
- Wrap `dlerror()` reads with guard to avoid stale error messages (`dlerror(); dlopen(...); dlerror();`).
- Log library path, resolved version, and function table size.
- Provide verbose mode to dump plugin metadata for debugging.

## Automation Prompt
```
You are assisting with "03. Dynamic Loading Mechanics" for the OpenHD plugin system manual. Generate POSIX and Windows pseudocode/templated code for a DynamicLibrary helper, include CMake snippets, and demonstrate sample usage. Ensure instructions are sequential and copy-paste friendly.
```
