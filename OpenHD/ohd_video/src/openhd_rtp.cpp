//
// Created by consti10 on 17.04.24.
//

#include "openhd_rtp.h"

#include <utility>

#include "openhd_util_time.h"
#include "rtp-profile.h"

static void* rtp_alloc(void* /*param*/, int bytes)
{
  static uint8_t buffer[2 * 1024 * 1024 + 4] = { 0, 0, 0, 1, };
  assert(bytes <= sizeof(buffer) - 4);
  return buffer + 4;
}

static void rtp_free(void* /*param*/, void * /*packet*/)
{
}

static int rtp_encode_packet(void* param, const void *packet, int bytes, uint32_t timestamp, int flags)
{
  auto self = (openhd::RTPHelper*)param;
  //openhd::log::get_default()->debug(" rtp_encode_packet {} {} {}",bytes,timestamp,flags);
  self->on_new_rtp_fragment((const uint8_t*)packet,bytes,timestamp,flags);
  // TODO
  return 0;
}



openhd::RTPHelper::RTPHelper(){
  m_console=openhd::log::create_or_get("RTPHelp");

  m_handler.alloc = rtp_alloc;
  m_handler.free = rtp_free;
  m_handler.packet = rtp_encode_packet;

  int payload=96; // WEIRD RTP_PAYLOAD_H264
  const char* encoding="H264";
  uint16_t seq=0;
  uint32_t ssrc=0;

  encoder = rtp_payload_encode_create(payload, encoding, seq, ssrc, &m_handler, this);
  assert(encoder);
}

void openhd::RTPHelper::feed_nalu(const uint8_t *data, int data_len) {
  m_console->debug("feed_nalu {}",data_len);
  int32_t timestamp=0;
  timestamp=openhd::util::steady_clock_time_epoch_ms();
  rtp_payload_encode_input(encoder, data, data_len, timestamp);
  // all frames processed
  m_console->debug("Done, got {} fragments",m_frame_fragments.size());
  if(m_out_cb){
    m_out_cb(m_frame_fragments);
  }
  m_frame_fragments.clear();
}


void openhd::RTPHelper::on_new_rtp_fragment(const uint8_t* data, int data_len,uint32_t timestamp,int last) {
  m_console->debug("on_new_rtp_fragment {} ts:{} last:{}",data_len,timestamp,last);
  auto shared=std::make_shared<std::vector<uint8_t>>(data,data+data_len);
  m_frame_fragments.emplace_back(shared);
}

void openhd::RTPHelper::set_out_cb(openhd::RTPHelper::OUT_CB cb) {
  m_out_cb=std::move(cb);
}
