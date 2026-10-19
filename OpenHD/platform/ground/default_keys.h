#pragma once
#include <sodium.h>
#include <optional>
#include <cstring>
#include "encryption/KeyPair.h"

namespace openhd::ground {
constexpr const char* kDefaultBindPhrase = "O-p-e-n-H-D";
static constexpr std::array<uint8_t, crypto_pwhash_SALTBYTES> kSaltAir{
    41, 129, 7, 201, 56, 11, 77, 90, 223, 5, 19, 41, 88, 93, 100, 11};
static constexpr std::array<uint8_t, crypto_pwhash_SALTBYTES> kSaltGnd{
    92, 18, 66, 220, 171, 9, 14, 207, 12, 44, 97, 2, 51, 99, 17, 221};

inline std::optional<wb::KeyPairTxRx> derive_default_wb_keypair() {
  if (sodium_init() == -1) {
    return std::nullopt;
  }

  std::array<uint8_t, crypto_box_SEEDBYTES> seed_air{};
  std::array<uint8_t, crypto_box_SEEDBYTES> seed_gnd{};

  if (crypto_pwhash(seed_air.data(), seed_air.size(), kDefaultBindPhrase,
                    std::strlen(kDefaultBindPhrase), kSaltAir.data(),
                    crypto_pwhash_OPSLIMIT_INTERACTIVE,
                    crypto_pwhash_MEMLIMIT_INTERACTIVE,
                    crypto_pwhash_ALG_DEFAULT) != 0) {
    return std::nullopt;
  }
  if (crypto_pwhash(seed_gnd.data(), seed_gnd.size(), kDefaultBindPhrase,
                    std::strlen(kDefaultBindPhrase), kSaltGnd.data(),
                    crypto_pwhash_OPSLIMIT_INTERACTIVE,
                    crypto_pwhash_MEMLIMIT_INTERACTIVE,
                    crypto_pwhash_ALG_DEFAULT) != 0) {
    return std::nullopt;
  }

  wb::KeyPairTxRx keypair{};
  crypto_box_seed_keypair(keypair.key_1.public_key.data(),
                          keypair.key_1.secret_key.data(), seed_air.data());
  crypto_box_seed_keypair(keypair.key_2.public_key.data(),
                          keypair.key_2.secret_key.data(), seed_gnd.data());
  return keypair;
}

}
