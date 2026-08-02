#include <cassert>
#include <iostream>

#include "openhd_plugin_manager.h"

int main(int argc, char* argv[]) {
  auto& manager = openhd::PluginManager::instance();
  const auto missing_loaded = manager.load_plugins(
      true, {"/definitely/not/a/real/openhd/plugin/directory"});
  assert(missing_loaded == 0);
  assert(manager.loaded_plugin_count() == 0);
  if (argc == 2) {
    const auto plugin_loaded = manager.load_plugins(true, {argv[1]});
    assert(plugin_loaded == 1);
    assert(manager.loaded_plugin_count() == 1);
  }
  manager.shutdown();
  std::cout << "Plugin loader test passed\n";
  return 0;
}
