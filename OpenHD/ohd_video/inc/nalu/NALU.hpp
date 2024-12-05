/******************************************************************************
 * OpenHD
 *
 * Licensed under the GNU General Public License (GPL) Version 3.
 *
 * This software is provided "as-is," without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose, and non-infringement. For details, see the
 * full license in the LICENSE file provided with this source code.
 *
 * Non-Military Use Only:
 * This software and its associated components are explicitly intended for
 * civilian and non-military purposes. Use in any military or defense
 * applications is strictly prohibited unless explicitly and individually
 * licensed otherwise by the OpenHD Team.
 *
 * Contributors:
 * A full list of contributors can be found at the OpenHD GitHub repository:
 * https://github.com/OpenHD
 *
 * © OpenHD, All Rights Reserved.
 ******************************************************************************/

#ifndef LIVE_VIDEO_10MS_ANDROID_NALU_H
#define LIVE_VIDEO_10MS_ANDROID_NALU_H

// https://github.com/Dash-Industry-Forum/Conformance-and-reference-source/blob/master/conformance/TSValidator/h264bitstream/h264_stream.h

#include <array>
#include <cassert>
#include <chrono>
#include <cstring>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include "NALUnitType.hpp"

// dependency could be easily removed again

static uint8_t extract_nal_unit_type(uint8_t value, bool is_h265) {
  if (is_h265) {
    return (value & 0x7E) >> 1;
  }
  return value & 0x1f;
}
static std::string x_get_nal_unit_type_as_string(int value, bool is_h265) {
  if (value < 0) return "{<0}";
  const uint8_t extracted = extract_nal_unit_type(value, is_h265);
  if (is_h265) {
    return NALUnitType::H265::unit_type_to_string(extracted);
  }
  return NALUnitType::H264::unit_type_to_string(extracted);
}
static bool is_idr_frame(int value, bool is_h265) {
  if (value <= 0) return false;
  const uint8_t extracted = extract_nal_unit_type(value, is_h265);
  if (is_h265) {
    return extracted == NALUnitType::H265::NAL_UNIT_CODED_SLICE_IDR_N_LP;
  }
  return extracted == NALUnitType::H264::NAL_UNIT_TYPE_CODED_SLICE_IDR;
}

/**
 * NOTE: NALU only takes a c-style data pointer - it does not do any memory
 * management. Use NALUBuffer if you need to store a NALU. Since H264 and H265
 * are that similar, we use this class for both (make sure to not call methds
 * only supported on h265 with a h264 nalu,though) The constructor of the NALU
 * does some really basic validation - make sure the parser never produces a
 * NALU where this validation would fail
 */
class NALU {
 public:
  NALU(const uint8_t* data1, size_t data_len1,
       const bool IS_H265_PACKET1 = false,
       const std::chrono::steady_clock::time_point creationTime =
           std::chrono::steady_clock::now())
      : m_data(data1),
        m_data_len(data_len1),
        IS_H265_PACKET(IS_H265_PACKET1),
        creationTime{creationTime} {
    assert(hasValidPrefix());
    assert(getSize() >= getMinimumNaluSize(IS_H265_PACKET1));
    m_nalu_prefix_size = get_nalu_prefix_size();
  }
  ~NALU() = default;
  // test video white iceland: Max 1024*117. Video might not be decodable if its
  // NALU buffers size exceed the limit But a buffer size of 1MB accounts for
  // 60fps video of up to 60MB/s or 480 Mbit/s. That should be plenty !
  static constexpr const auto NALU_MAXLEN = 1024 * 1024;
  // Application should re-use NALU_BUFFER to avoid memory allocations
  using NALU_BUFFER = std::array<uint8_t, NALU_MAXLEN>;

 private:
  const uint8_t* m_data;
  const size_t m_data_len;
  int m_nalu_prefix_size;

 public:
  const bool IS_H265_PACKET;
  // creation time is used to measure latency
  const std::chrono::steady_clock::time_point creationTime;

 public:
  // returns true if starts with 0001, false otherwise
  bool hasValidPrefixLong() const {
    return m_data[0] == 0 && m_data[1] == 0 && m_data[2] == 0 && m_data[3] == 1;
  }
  // returns true if starts with 001 (short prefix), false otherwise
  bool hasValidPrefixShort() const {
    return m_data[0] == 0 && m_data[1] == 0 && m_data[2] == 1;
  }
  bool hasValidPrefix() const {
    return hasValidPrefixLong() || hasValidPrefixShort();
  }
  int get_nalu_prefix_size() const {
    if (hasValidPrefixLong()) return 4;
    return 3;
  }
  static std::size_t getMinimumNaluSize(const bool isH265) {
    // 4 bytes prefix, 1 byte header for h264, 2 byte header for h265
    return isH265 ? 6 : 5;
  }

 public:
  // pointer to the NALU data with 0001 prefix
  const uint8_t* getData() const { return m_data; }
  // size of the NALU data with 0001 prefix
  size_t getSize() const { return m_data_len; }
  // pointer to the NALU data without 0001 prefix
  const uint8_t* getDataWithoutPrefix() const {
    return &getData()[m_nalu_prefix_size];
  }
  // size of the NALU data without 0001 prefix
  ssize_t getDataSizeWithoutPrefix() const {
    return getSize() - m_nalu_prefix_size;
  }
  // return the nal unit type (quick)
  int get_nal_unit_type() const {
    if (IS_H265_PACKET) {
      return (getDataWithoutPrefix()[0] & 0x7E) >> 1;
    }
    return getDataWithoutPrefix()[0] & 0x1f;
  }
  std::string get_nal_unit_type_as_string() const {
    if (IS_H265_PACKET) {
      return NALUnitType::H265::unit_type_to_string(get_nal_unit_type());
    }
    return NALUnitType::H264::unit_type_to_string(get_nal_unit_type());
  }

 public:
  bool isSPS() const {
    if (IS_H265_PACKET) {
      return get_nal_unit_type() == NALUnitType::H265::NAL_UNIT_SPS;
    }
    return (get_nal_unit_type() == NALUnitType::H264::NAL_UNIT_TYPE_SPS);
  }
  bool isPPS() const {
    if (IS_H265_PACKET) {
      return get_nal_unit_type() == NALUnitType::H265::NAL_UNIT_PPS;
    }
    return (get_nal_unit_type() == NALUnitType::H264::NAL_UNIT_TYPE_PPS);
  }
  // VPS NALUs are only possible in H265
  bool isVPS() const {
    assert(IS_H265_PACKET);
    return get_nal_unit_type() == NALUnitType::H265::NAL_UNIT_VPS;
  }
  bool is_aud() const {
    if (IS_H265_PACKET) {
      return get_nal_unit_type() ==
             NALUnitType::H265::NAL_UNIT_ACCESS_UNIT_DELIMITER;
    }
    return (get_nal_unit_type() == NALUnitType::H264::NAL_UNIT_TYPE_AUD);
  }
  bool is_sei() const {
    if (IS_H265_PACKET) {
      return get_nal_unit_type() == NALUnitType::H265::NAL_UNIT_PREFIX_SEI ||
             get_nal_unit_type() == NALUnitType::H265::NAL_UNIT_SUFFIX_SEI;
    }
    return (get_nal_unit_type() == NALUnitType::H264::NAL_UNIT_TYPE_SEI);
  }
  bool is_dps() const {
    if (IS_H265_PACKET) {
      // doesn't exist in h265
      return false;
    }
    return (get_nal_unit_type() == NALUnitType::H264::NAL_UNIT_TYPE_DPS);
  }
  bool is_config() const {
    return isSPS() || isPPS() || (IS_H265_PACKET && isVPS());
  }
  // keyframe / IDR frame
  bool is_keyframe() const {
    const auto nut = get_nal_unit_type();
    if (IS_H265_PACKET) {
      return false;
    }
    if (nut == NALUnitType::H264::NAL_UNIT_TYPE_CODED_SLICE_IDR) {
      return true;
    }
    return false;
  }
  bool is_frame_but_not_keyframe() const {
    const auto nut = get_nal_unit_type();
    if (IS_H265_PACKET) return false;
    return (nut == NALUnitType::H264::NAL_UNIT_TYPE_CODED_SLICE_NON_IDR);
  }
  static bool has_valid_prefix(const uint8_t* p) {
    return p[0] == 0 && p[1] == 0 && p[2] == 0 && p[3] == 1 ||
           p[0] == 0 && p[1] == 0 && p[2] == 1;
  }
  static void write_prefix(uint8_t* p, bool long_prefix) {
    if (long_prefix) {
      p[0] = 0;
      p[1] = 0;
      p[2] = 0;
      p[3] = 1;
    } else {
      p[0] = 0;
      p[1] = 0;
      p[2] = 1;
    }
  }
};

// Copies the nalu data into its own c++-style managed buffer.
class NALUBuffer {
 public:
  NALUBuffer(const uint8_t* data, int data_len, bool is_h265,
             std::chrono::steady_clock::time_point creation_time) {
    m_data = std::make_shared<std::vector<uint8_t>>(data, data + data_len);
    m_nalu = std::make_unique<NALU>(m_data->data(), m_data->size(), is_h265,
                                    creation_time);
  }
  explicit NALUBuffer(const NALU& nalu) {
    m_data = std::make_shared<std::vector<uint8_t>>(
        nalu.getData(), nalu.getData() + nalu.getSize());
    m_nalu = std::make_shared<NALU>(m_data->data(), m_data->size(),
                                    nalu.IS_H265_PACKET, nalu.creationTime);
  }
  NALUBuffer(const NALUBuffer&) = delete;
  NALUBuffer(const NALUBuffer&&) = delete;

  const NALU& get_nal() { return *m_nalu; }

 private:
  std::shared_ptr<std::vector<uint8_t>> m_data;
  std::shared_ptr<NALU> m_nalu;
};

#endif  // LIVE_VIDEO_10MS_ANDROID_NALU_H
