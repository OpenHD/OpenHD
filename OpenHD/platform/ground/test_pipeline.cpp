#include "encryption/Encryption.h"
#include "encryption/Encryptor.h"
#include "encryption/Decryptor.h"
#include "encryption/EncryptionFsUtils.h"
#include "fec/FECEncoder.h"
#include "fec/FECDecoder.h"
#include "Ieee80211Header.hpp"
#include "default_keys.h"
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <iostream>
#include <stdexcept>

void require(bool value, const char* message) {
  if (!value) throw std::runtime_error(message);
}
int main() {
  try {
    require(sodium_init() >= 0, "sodium initialization failed");
    const auto defaults = openhd::ground::derive_default_wb_keypair();
    require(bool(defaults), "Default key derivation failed");
    std::array<uint8_t, 32> digest{};
    crypto_generichash(digest.data(), digest.size(), reinterpret_cast<const uint8_t*>(&*defaults), sizeof(*defaults), nullptr, 0);
    std::ostringstream digest_text;
    for (const auto byte : digest) digest_text << std::hex << std::setfill('0') << std::setw(2) << int(byte);
    require(digest_text.str() == "dd3d97f87369ee59ac4077dfbf721a71f51f7bf7335c320eb6ecc39f9bc05498",
            "Built-in key differs from the Linux OpenHD default");
    SequenceControl sequence{};
    sequence.subfield = 0xa;
    sequence.sequence_nr = 0x123;
    const auto* wire = reinterpret_cast<const uint8_t*>(&sequence);
    require(wire[0] == 0x3a && wire[1] == 0x12, "802.11 bitfield wire layout differs from Linux");
    const auto keys = wb::generate_keypair_random();
    const auto key = keys.key_1;
    wb::Encryptor encrypt(key);
    wb::Decryptor decrypt(key);
    std::array<uint8_t, crypto_box_NONCEBYTES> session_nonce{};
    std::array<uint8_t, crypto_aead_chacha20poly1305_KEYBYTES + crypto_box_MACBYTES> session{};
    encrypt.makeNewSessionKey(session_nonce, session);
    require(decrypt.onNewPacketSessionKeyData(session_nonce, session) == 0, "Session rejected");
    FECEncoder encoder;
    FECDecoder decoder(1);
    std::vector<std::vector<uint8_t>> encoded, decoded;
    std::vector<std::shared_ptr<std::vector<uint8_t>>> payloads;
    for (int i = 0; i < 3; ++i) {
      auto p = std::make_shared<std::vector<uint8_t>>(120 + i * 10, uint8_t(i + 10));
      p->at(0) = 0; p->at(1) = 13; p->at(2) = 26; p->at(3) = 255;
      payloads.push_back(p);
    }
    encoder.m_out_cb = [&](const uint8_t* p, int n) { encoded.emplace_back(p, p + n); };
    decoder.mSendDecodedPayloadCallback = [&](const uint8_t* p, std::size_t n) {
      decoded.emplace_back(p, p + n);
    };
    encoder.encode_block(payloads, 2);
    require(encoded.size() == 5, "FEC encoder count");
    for (std::size_t i : {0u, 2u, 3u}) {  // Drop primary fragment 1; recover using parity.
      const uint64_t nonce = 100 + i;
      const auto ciphertext = encrypt.authenticate_and_encrypt_buff(nonce, encoded[i].data(), encoded[i].size());
      auto damaged = *ciphertext;
      damaged.back() ^= 1;
      require(!decrypt.authenticate_and_decrypt_buff(nonce, damaged.data(), damaged.size(), true),
              "Corrupted packet accepted");
      const auto plaintext = decrypt.authenticate_and_decrypt_buff(nonce, ciphertext->data(), ciphertext->size(), true);
      require(bool(plaintext), "Valid ciphertext rejected");
      require(decoder.process_valid_packet(plaintext->data(), plaintext->size()), "FEC fragment rejected");
    }
    require(decoded.size() == payloads.size(), "FEC recovery count");
    for (std::size_t i = 0; i < payloads.size(); ++i)
      require(decoded[i] == *payloads[i], "Recovered payload differs");
    auto binary_keys = keys;
    binary_keys.key_1.public_key[0] = 10;
    binary_keys.key_1.public_key[1] = 26;
    const auto filename = std::filesystem::temp_directory_path() /
        ("openhd-key-test-" + std::to_string(randombytes_random()) + ".bin");
    require(wb::write_keypair_to_file(binary_keys, filename.string()), "Key write failed");
    const auto loaded = wb::read_keypair_from_file(filename.string());
    std::filesystem::remove(filename);
    require(loaded && *loaded == binary_keys, "Windows binary keypair roundtrip failed");
    std::cout << "Encryption, tamper rejection, FEC loss recovery and binary keys passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
