//
// Created by consti10 on 06.12.22.
//

#include "rtp_eof_helper.h"

// Look here for more details (or just look into the rtp rfc:
// https://github.com/Consti10/LiveVideo10ms/tree/99e2c4ca31dd8c446952cd409ed51f798e29a137/VideoCore/src/main/cpp/Parser
static constexpr auto RTP_HEADER_SIZE = 12;
namespace H264 {
struct nalu_header_t {
  uint8_t type : 5;
  uint8_t nri : 2;
  uint8_t f : 1;
} __attribute__((packed));
struct fu_header_t {
  uint8_t type : 5;
  uint8_t r : 1;
  uint8_t e : 1;
  uint8_t s : 1;
} __attribute__((packed));
}
namespace H265 {
struct nal_unit_header_h265_t {
  uint8_t f : 1;
  uint8_t type : 6;
  uint8_t layerId : 6;
  uint8_t tid : 3;
} __attribute__((packed));
static_assert(sizeof(nal_unit_header_h265_t) == 2);
struct fu_header_h265_t {
  uint8_t fuType : 6;
  uint8_t e : 1;
  uint8_t s : 1;
} __attribute__((packed));
static_assert(sizeof(fu_header_h265_t) == 1);
}

bool openhd::rtp_eof_helper::h264_end_block(const uint8_t *payload,
                                       const std::size_t payloadSize) {
  if (payloadSize < RTP_HEADER_SIZE + sizeof(H264::nalu_header_t)) {
    std::cerr << "Got packet that cannot be rtp h264\n";
    return false;
  }
  const H264::nalu_header_t &naluHeader = *(H264::nalu_header_t *) (&payload[RTP_HEADER_SIZE]);
  if (naluHeader.type == 28) {// fragmented nalu
    if (payloadSize < RTP_HEADER_SIZE + sizeof(H264::nalu_header_t) + sizeof(H264::fu_header_t)) {
      std::cerr << "Got invalid h264 rtp fu packet\n";
      return false;
    }
    //std::cout<<"Got fragmented NALU\n";
    const H264::fu_header_t &fuHeader = *(H264::fu_header_t *) &payload[RTP_HEADER_SIZE + sizeof(H264::nalu_header_t)];
    if (fuHeader.e) {
      //std::cout<<"Got end of fragmented NALU\n";
      // end of fu-a
      return true;
    } else {
      //std::cout<<"Got start or middle of fragmented NALU\n";
      return false;
    }
  } else if (naluHeader.type > 0 && naluHeader.type < 24) {//full nalu
    //std::cout<<"Got full NALU\n";
    return true;
  } else {
    std::cerr << "Unknown rtp h264 packet\n";
    return true;
  }
}

bool openhd::rtp_eof_helper::h265_end_block(const uint8_t *payload,
                                       const std::size_t payloadSize) {
  if (payloadSize < RTP_HEADER_SIZE + sizeof(H265::nal_unit_header_h265_t)) {
    std::cerr << "Got packet that cannot be rtp h265\n";
    return false;
  }
  const H265::nal_unit_header_h265_t &naluHeader = *(H265::nal_unit_header_h265_t *) (&payload[RTP_HEADER_SIZE]);
  if (naluHeader.type == 49) {
    if (payloadSize < RTP_HEADER_SIZE + sizeof(H265::nal_unit_header_h265_t) + sizeof(H265::fu_header_h265_t)) {
      std::cerr << "Got invalid h265 rtp fu packet\n";
      return false;
    }
    const H265::fu_header_h265_t
        &fuHeader = *(H265::fu_header_h265_t *) &payload[RTP_HEADER_SIZE + sizeof(H265::nal_unit_header_h265_t)];
    if (fuHeader.e) {
      //std::cout<<"Got end of fragmented NALU\n";
      // end of fu-a
      return true;
    } else {
      //std::cout<<"Got start or middle of fragmented NALU\n";
      return false;
    }
  } else {
    //std::cout<<"Got h265 nalu that is not a fragmentation unit\n";
    return true;
  }
}

bool openhd::rtp_eof_helper::mjpeg_end_block(const uint8_t *payload,
                                        const std::size_t payloadSize) {
  // TODO not yet supported
  return false;
}
