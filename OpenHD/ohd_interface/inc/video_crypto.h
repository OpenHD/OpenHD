#ifndef OPENHD_VIDEO_CRYPTO_H
#define OPENHD_VIDEO_CRYPTO_H

#include <cstddef>
#include <memory>
#include <string>

namespace spdlog {
class logger;
}

namespace openhd {

class VideoCrypto {
 public:
  explicit VideoCrypto(std::shared_ptr<spdlog::logger> logger);
  VideoCrypto(const VideoCrypto&) = delete;
  VideoCrypto& operator=(const VideoCrypto&) = delete;
  ~VideoCrypto();

  bool load(bool is_air);
  bool is_loaded() const { return m_handle != nullptr; }
  const std::string& loaded_path() const { return m_loaded_path; }
  bool get_wb_keypair(uint8_t* out, size_t out_len) const;

 private:
  std::shared_ptr<spdlog::logger> m_console;
  void* m_handle = nullptr;
  std::string m_loaded_path;
  using init_fn = int (*)(int is_air);
  using shutdown_fn = void (*)();
  using get_wb_keypair_fn = int (*)(uint8_t* out, size_t out_len);
  init_fn m_init = nullptr;
  shutdown_fn m_shutdown = nullptr;
  get_wb_keypair_fn m_get_wb_keypair = nullptr;
};

}  // namespace openhd

#endif  // OPENHD_VIDEO_CRYPTO_H
