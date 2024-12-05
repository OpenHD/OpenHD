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

#ifndef LIVEVIDEO10MS_KEYFRAMEFINDER_HPP
#define LIVEVIDEO10MS_KEYFRAMEFINDER_HPP

#include <memory>
#include <vector>

#include "NALU.hpp"
// #include <qdebug.h>
#include <array>

// Takes a continuous stream of NALUs and save SPS / PPS data
// For later use
class CodecConfigFinder {
 private:
  std::unique_ptr<NALUBuffer> SPS = nullptr;
  std::unique_ptr<NALUBuffer> PPS = nullptr;
  // VPS are only used in H265
  std::unique_ptr<NALUBuffer> VPS = nullptr;

 public:
  bool save_if_config(const NALU& nalu) {
    if (nalu.getSize() <= 0) return false;
    if (nalu.isSPS()) {
      SPS = std::make_unique<NALUBuffer>(nalu);
      // qDebug()<<"SPS found";
      // qDebug()<<nalu.get_sps_as_string().c_str();
      return true;
    } else if (nalu.isPPS()) {
      PPS = std::make_unique<NALUBuffer>(nalu);
      // qDebug()<<"PPS found";
      return true;
    } else if (nalu.IS_H265_PACKET && nalu.isVPS()) {
      VPS = std::make_unique<NALUBuffer>(nalu);
      // qDebug()<<"VPS found";
      return true;
    }
    // qDebug()<<"not a keyframe"<<(int)nalu.getDataWithoutPrefix()[0];
    return false;
  }
  // H264 needs sps and pps
  // H265 needs sps,pps and vps
  bool all_config_available(const bool IS_H265 = false) {
    if (IS_H265) {
      return SPS != nullptr && PPS != nullptr && VPS != nullptr;
    }
    return SPS != nullptr && PPS != nullptr;
  }
  std::shared_ptr<std::vector<uint8_t>> get_config_data(
      const bool IS_H265 = false) {
    assert(all_config_available(IS_H265));
    if (IS_H265) {
      // Looks like avcodec wants the VPS before sps and pps
      auto& sps = SPS->get_nal();
      auto& pps = PPS->get_nal();
      auto& vps = VPS->get_nal();
      const auto size = sps.getSize() + pps.getSize() + vps.getSize();
      auto ret = std::make_unique<std::vector<uint8_t>>(size);
      std::memcpy(ret->data(), vps.getData(), vps.getSize());
      auto offset = vps.getSize();
      std::memcpy(ret->data() + offset, sps.getData(), sps.getSize());
      offset += sps.getSize();
      std::memcpy(ret->data() + offset, pps.getData(), pps.getSize());
      return ret;
    }
    auto& sps = SPS->get_nal();
    auto& pps = PPS->get_nal();
    const auto size = sps.getSize() + pps.getSize();
    auto ret = std::make_shared<std::vector<uint8_t>>(size);
    std::memcpy(ret->data(), sps.getData(), sps.getSize());
    std::memcpy(ret->data() + sps.getSize(), pps.getData(), pps.getSize());
    return ret;
  }
  // returns false if the config data (SPS,PPS,optional VPS) has changed
  // true otherwise
  bool check_is_still_same_config_data(const NALU& nalu) {
    assert(all_config_available(nalu.IS_H265_PACKET));
    if (nalu.isSPS()) {
      return compare(nalu, SPS->get_nal());
    } else if (nalu.isPPS()) {
      return compare(nalu, PPS->get_nal());
    } else if (nalu.IS_H265_PACKET && nalu.isVPS()) {
      return compare(nalu, VPS->get_nal());
    }
    return true;
  }
  static void appendNaluData(std::vector<uint8_t>& buff, const NALU& nalu) {
    buff.insert(buff.begin(), nalu.getData(), nalu.getData() + nalu.getSize());
  }
  void reset() {
    SPS = nullptr;
    PPS = nullptr;
    VPS = nullptr;
  }
  const NALU& getCSD0() const { return SPS->get_nal(); }
  const NALU& getCSD1() const { return PPS->get_nal(); }
  const NALU& getVPS() const { return VPS->get_nal(); }

 public:
  static bool compare(const NALU& n1, const NALU& n2) {
    if (n1.getSize() != n2.getSize()) return false;
    const int res = std::memcmp(n1.getData(), n2.getData(), n1.getSize());
    return res == 0;
  }
};

#endif  // LIVEVIDEO10MS_KEYFRAMEFINDER_HPP
