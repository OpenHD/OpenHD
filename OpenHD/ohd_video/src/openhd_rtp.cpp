//
// Created by consti10 on 17.04.24.
//

#include "openhd_rtp.h"

#include <utility>

#include "nalu/CodecConfigFinder.hpp"
#include "nalu/fragment_helper.h"
#include "nalu/nalu_helper.h"
#include "openhd_util_time.h"
#include "rtp-profile.h"
#include "rtp_eof_helper.h"

static void* rtp_alloc(void* /*param*/, int bytes) {
  static uint8_t buffer[2 * 1024 * 1024 + 4] = {
      0,
      0,
      0,
      1,
  };
  assert(bytes <= sizeof(buffer) - 4);
  return buffer + 4;
}

static void rtp_free(void* /*param*/, void* /*packet*/) {}

static int rtp_encode_packet(void* param, const void* packet, int bytes,
                             uint32_t timestamp, int flags) {
  auto self = (openhd::RTPHelper*)param;
  // openhd::log::get_default()->debug(" rtp_encode_packet {} {}
  // {}",bytes,timestamp,flags);
  self->on_new_rtp_fragment((const uint8_t*)packet, bytes, timestamp, flags);
  // TODO
  return 0;
}

openhd::RTPHelper::RTPHelper(bool is_h265) : m_is_h265(is_h265) {
  m_console = openhd::log::create_or_get("RTPHelp");

  m_handler.alloc = rtp_alloc;
  m_handler.free = rtp_free;
  m_handler.packet = rtp_encode_packet;

  int payload = 96;  // WEIRD RTP_PAYLOAD_H264
  const char* encoding = "H264";
  if (m_is_h265) {
    encoding = "H265";
  }
  uint16_t seq = 0;
  uint32_t ssrc = 0;

  encoder =
      rtp_payload_encode_create(payload, encoding, seq, ssrc, &m_handler, this);
  assert(encoder);
}

openhd::RTPHelper::~RTPHelper() { rtp_payload_encode_destroy(encoder); }

void openhd::RTPHelper::feed_multiple_nalu(const uint8_t* data, int data_len) {
  int offset = 0;
  while (offset < data_len) {
    int nalu_len = find_next_nal(&data[offset], data_len);
    on_new_split_nalu(&data[offset], nalu_len);
    offset += nalu_len;
  }
}

void openhd::RTPHelper::feed_nalu(const uint8_t* data, int data_len) {
  // m_console->debug("feed_nalu {}", data_len);
  int32_t timestamp = 0;
  timestamp = openhd::util::steady_clock_time_epoch_ms();
  rtp_payload_encode_input(encoder, data, data_len, timestamp);
  // all frames processed
  // m_console->debug("Done, got {} fragments", m_frame_fragments.size());
  if (m_out_cb) {
    m_out_cb(m_frame_fragments);
  }
  m_frame_fragments.clear();
}

void openhd::RTPHelper::on_new_rtp_fragment(const uint8_t* data, int data_len,
                                            uint32_t timestamp, int last) {
  // m_console->debug("on_new_rtp_fragment {} ts:{} last:{}", data_len,
  // timestamp,
  //                  last);
  auto shared = std::make_shared<std::vector<uint8_t>>(data, data + data_len);
  m_frame_fragments.emplace_back(shared);
}

void openhd::RTPHelper::set_out_cb(openhd::RTPHelper::OUT_CB cb) {
  m_out_cb = std::move(cb);
}

void openhd::RTPHelper::on_new_split_nalu(const uint8_t* data, int data_len) {
  NALU nalu(data, data_len);
  // m_console->debug("Got new NAL {}
  // {}",data_len,nalu.get_nal_unit_type_as_string()); if(nalu.is_sei())return;
  if (m_config_finder.all_config_available(m_is_h265)) {
    if (nalu.is_config()) {
      if (m_config_finder.check_is_still_same_config_data(nalu)) {
        // we can discard this NAL
      } else {
        m_config_finder.reset();
        m_config_finder.save_if_config(nalu);
      }
    } else {
      if (nalu.is_sei() || nalu.is_aud()) {
        // We can discard (AUDs are written manually on the rx)
      } else {
        on_new_nalu_frame(data, data_len);
      }
    }
  } else {
    m_config_finder.save_if_config(nalu);
  }
}

void openhd::RTPHelper::on_new_nalu_frame(const uint8_t* data, int data_len) {
  // Wait until we have codec config
  if (!m_config_finder.all_config_available(m_is_h265)) {
    return;
  }
  const auto elapsed_last_config =
      std::chrono::steady_clock::now() - m_last_codec_config_send_ts;
  if (elapsed_last_config >= std::chrono::seconds(1)) {
    m_last_codec_config_send_ts = std::chrono::steady_clock::now();
    auto config = m_config_finder.get_config_data(m_is_h265);
    feed_nalu(config->data(), config->size());
  }
  feed_nalu(data, data_len);
}

openhd::RTPFragmentBuffer::RTPFragmentBuffer() {
  m_console = openhd::log::create_or_get("RTPFragmentBuffer");
}

void openhd::RTPFragmentBuffer::buffer_and_forward(
    std::shared_ptr<std::vector<uint8_t>> fragment, uint64_t dts) {
  m_frame_fragments.push_back(fragment);
  openhd::rtp_eof_helper::RTPFragmentInfo info{};
  if (m_is_h265) {
    info = openhd::rtp_eof_helper::h265_more_info(fragment->data(),
                                                  fragment->size());
  } else {
    info = openhd::rtp_eof_helper::h264_more_info(fragment->data(),
                                                  fragment->size());
  }
  if (info.is_fu_start) {
    if (is_idr_frame(info.nal_unit_type, m_is_h265)) {
      m_last_fu_s_idr = true;
    } else {
      m_last_fu_s_idr = false;
    }
  }
  // m_console->debug("Fragment {} start:{} end:{}
  // type:{}",m_frame_fragments.size(),
  //                  OHDUtil::yes_or_no(info.is_fu_start),
  //                  OHDUtil::yes_or_no(info.is_fu_end),
  //                  x_get_nal_unit_type_as_string(info.nal_unit_type,is_h265));
  bool is_last_fragment_of_frame = info.is_fu_end;
  if (m_frame_fragments.size() > 500) {
    // Most likely something wrong with the "find end of frame" workaround
    m_console->debug("No end of frame found after 1000 fragments");
    is_last_fragment_of_frame = true;
  }
  if (is_last_fragment_of_frame) {
    on_new_rtp_fragmented_frame();
    m_frame_fragments.resize(0);
    m_last_fu_s_idr = false;
  }
}

void openhd::RTPFragmentBuffer::on_new_rtp_fragmented_frame() {
  const bool is_intra_frame = m_last_fu_s_idr;
  auto frame = openhd::FragmentedVideoFrame{m_frame_fragments,
                                            std::chrono::steady_clock::now(),
                                            m_enable_ultra_secure_encryption,
                                            nullptr,
                                            m_uses_intra_refresh,
                                            is_intra_frame};
  // m_console->debug("{}",frame.to_string());
}
