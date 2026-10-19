#pragma once

#include <algorithm>
#include <functional>

#include "openhd_video_frame.h"
#include "video_worker_ipc.h"

namespace openhd::video_worker {
inline bool send_frame(int fd, uint32_t sequence,
                       const FragmentedVideoFrame& frame) {
  Header header{Type::FrameBegin, sequence};
  header.flags = (frame.enable_ultra_secure_encryption ? 1u : 0u) |
                 (frame.is_intra_stream ? 2u : 0u) |
                 (frame.is_idr_frame ? 4u : 0u);
  header.created_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                          frame.creation_time.time_since_epoch())
                          .count();
  if (!send_packet(fd, header)) return false;
  for (const auto& fragment : frame.rtp_fragments) {
    if (!fragment || !send_packet(fd, {Type::Fragment, sequence},
                                  fragment->data(), fragment->size()))
      return false;
  }
  if (frame.dirty_frame) {
    for (std::size_t offset = 0; offset < frame.dirty_frame->size();
         offset += kChunkSize) {
      auto size = std::min(kChunkSize, frame.dirty_frame->size() - offset);
      if (!send_packet(fd, {Type::Raw, sequence},
                       frame.dirty_frame->data() + offset, size))
        return false;
    }
  }
  // A congested socket drops the entire frame. No incomplete frame is exposed
  // to the radio; the next FrameBegin discards any partial assembly.
  return send_packet(fd, {Type::FrameEnd, sequence});
}

class FrameAssembler {
 public:
  bool accept(const Header& header, const std::vector<uint8_t>& data) {
    if (header.type == Type::FrameBegin) {
      frame = {};
      m_bytes = 0;
      m_active = true;
      m_sequence = header.sequence;
      frame.creation_time = std::chrono::steady_clock::time_point(
          std::chrono::duration_cast<std::chrono::steady_clock::duration>(
              std::chrono::nanoseconds(header.created_ns)));
      frame.enable_ultra_secure_encryption = header.flags & 1;
      frame.is_intra_stream = header.flags & 2;
      frame.is_idr_frame = header.flags & 4;
      return false;
    }
    if (!m_active || header.sequence != m_sequence) return false;
    if (header.type == Type::FrameEnd) {
      m_active = false;
      return m_bytes != 0;
    }
    if (header.type != Type::Raw && header.type != Type::Fragment) return false;
    m_bytes += data.size();
    if (m_bytes > kMaxFrameSize || frame.rtp_fragments.size() >= 4096) {
      m_active = false;
      frame = {};
      return false;
    }
    if (header.type == Type::Fragment) {
      frame.rtp_fragments.push_back(
          std::make_shared<std::vector<uint8_t>>(data));
    } else {
      if (!frame.dirty_frame)
        frame.dirty_frame = std::make_shared<std::vector<uint8_t>>();
      frame.dirty_frame->insert(frame.dirty_frame->end(), data.begin(),
                                data.end());
    }
    return false;
  }
  FragmentedVideoFrame frame;

 private:
  bool m_active = false;
  uint32_t m_sequence = 0;
  std::size_t m_bytes = 0;
};
}  // namespace openhd::video_worker
