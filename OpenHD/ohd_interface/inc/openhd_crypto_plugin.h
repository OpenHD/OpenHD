/******************************************************************************
 * Versioned C ABI for independently-built OpenHD video crypto providers.
 ******************************************************************************/

#ifndef OPENHD_CRYPTO_PLUGIN_H
#define OPENHD_CRYPTO_PLUGIN_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OPENHD_CRYPTO_ABI_VERSION 1U
#define OPENHD_CRYPTO_ENTRYPOINT "openhd_crypto_get_provider"

struct openhd_crypto_provider {
  uint32_t struct_size;
  uint32_t abi_version;
  uint32_t wire_format_id;
  uint32_t max_ciphertext_overhead;
  const char* name;
  const char* version;

  int32_t (*init)(int32_t is_air);
  void (*shutdown)(void);
  int32_t (*encrypt)(const uint8_t* input, size_t input_len, uint8_t* output,
                     size_t output_capacity, size_t* output_len,
                     uint64_t* key_id);
  int32_t (*decrypt)(const uint8_t* input, size_t input_len, uint8_t* output,
                     size_t output_capacity, size_t* output_len,
                     uint64_t key_id);
};

typedef const struct openhd_crypto_provider* (*openhd_crypto_get_provider_fn)(
    void);

#ifdef __cplusplus
}
#endif

#endif  // OPENHD_CRYPTO_PLUGIN_H
