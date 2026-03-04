#include "video_crypto.h"

#include <cstdlib>
#include <string>

#include "openhd_spdlog.h"
#include "openhd_util_filesystem.h"

#ifdef __linux__
#include <dlfcn.h>
#endif

namespace {
constexpr const char* kVideoCryptoEnvVar = "OPENHD_VIDEO_CRYPTO_SO";
constexpr const char* kDefaultVideoCryptoPath =
    "/usr/local/lib/openhd/libohd_video_crypto.so";
}  // namespace

openhd::VideoCrypto::VideoCrypto(std::shared_ptr<spdlog::logger> logger)
    : m_console(std::move(logger)) {}

openhd::VideoCrypto::~VideoCrypto() {
#ifdef __linux__
  if (m_shutdown) {
    m_shutdown();
  }
  if (m_handle) {
    dlclose(m_handle);
  }
#endif
}

bool openhd::VideoCrypto::load(bool is_air) {
#ifdef __linux__
  if (m_handle != nullptr) {
    return true;
  }
  const char* env_path = std::getenv(kVideoCryptoEnvVar);
  const std::string path =
      (env_path && env_path[0] != '\0') ? env_path : kDefaultVideoCryptoPath;
  if (!OHDFilesystemUtil::exists(path)) {
    m_console->debug("Video crypto library not found at {}", path);
    return false;
  }
  m_handle = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
  if (!m_handle) {
    m_console->error("Failed to load video crypto library: {}",
                     dlerror() ? dlerror() : "unknown error");
    return false;
  }
  m_init =
      reinterpret_cast<init_fn>(dlsym(m_handle, "openhd_video_crypto_init"));
  m_shutdown = reinterpret_cast<shutdown_fn>(
      dlsym(m_handle, "openhd_video_crypto_shutdown"));
  m_get_wb_keypair = reinterpret_cast<get_wb_keypair_fn>(
      dlsym(m_handle, "openhd_video_crypto_get_wb_keypair"));
  if (!m_init || !m_shutdown) {
    m_console->error(
        "Video crypto library missing required symbols "
        "(openhd_video_crypto_init/openhd_video_crypto_shutdown)");
    dlclose(m_handle);
    m_handle = nullptr;
    m_init = nullptr;
    m_shutdown = nullptr;
    return false;
  }
  const int rc = m_init(is_air ? 1 : 0);
  if (rc != 0) {
    m_console->error("Video crypto init failed with code {}", rc);
    dlclose(m_handle);
    m_handle = nullptr;
    m_init = nullptr;
    m_shutdown = nullptr;
    return false;
  }
  m_loaded_path = path;
  m_console->info("Loaded video crypto library {}", path);
  return true;
#else
  (void)is_air;
  return false;
#endif
}

bool openhd::VideoCrypto::get_wb_keypair(uint8_t* out,
                                         size_t out_len) const {
  if (!m_get_wb_keypair) {
    return false;
  }
  if (!out || out_len == 0) {
    return false;
  }
  return m_get_wb_keypair(out, out_len) == 0;
}
