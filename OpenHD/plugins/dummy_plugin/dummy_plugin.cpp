#include "plugins/plugins.h"

#include "openhd_spdlog.h"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <memory>
#include <thread>

#include <spdlog/spdlog.h>

namespace {

struct DummyPluginContext {
  std::shared_ptr<spdlog::logger> logger;
  std::atomic<bool> running{false};
  std::thread worker;
};

void run_worker(DummyPluginContext *context) {
  using namespace std::chrono_literals;
  while (context->running.load(std::memory_order_acquire)) {
    context->logger->debug("Dummy plugin says: hello world");
    for (int i = 0; i < 50 &&
                    context->running.load(std::memory_order_acquire);
         ++i) {
      std::this_thread::sleep_for(100ms);
    }
  }
}

}  // namespace

extern "C" {

struct openhd_plugin_context {
  DummyPluginContext impl;
};

const struct openhd_plugin_info *openhd_plugin_get_info() {
  static const struct openhd_plugin_info kInfo = {
      OPENHD_PLUGIN_API_VERSION,
      "dummy_hello_plugin",
      "Logs a hello world message every five seconds.",
      openhd_plugin_type::UNKNOWN,
      0,
  };
  return &kInfo;
}

struct openhd_plugin_context *openhd_plugin_init(void * /*host_context*/) {
  auto *context = new openhd_plugin_context();
  context->impl.logger = openhd::log::create_or_get("dummy_plugin");
  context->impl.logger->set_level(spdlog::level::debug);
  context->impl.logger->debug("Initializing dummy plugin");
  context->impl.running.store(true, std::memory_order_release);
  context->impl.worker = std::thread(run_worker, &context->impl);
  return context;
}

void openhd_plugin_shutdown(struct openhd_plugin_context *context) {
  if (!context) {
    return;
  }
  context->impl.logger->debug("Shutting down dummy plugin");
  context->impl.running.store(false, std::memory_order_release);
  if (context->impl.worker.joinable()) {
    context->impl.worker.join();
  }
  delete context;
}

}  // extern "C"
