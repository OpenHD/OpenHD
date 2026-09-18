#include "matroska_recorder.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <limits>

namespace {

using Bytes = std::vector<uint8_t>;

void append_id(Bytes& out, uint32_t id) {
  int bytes = 1;
  if (id > 0xFFFFFF) {
    bytes = 4;
  } else if (id > 0xFFFF) {
    bytes = 3;
  } else if (id > 0xFF) {
    bytes = 2;
  }
  for (int shift = (bytes - 1) * 8; shift >= 0; shift -= 8) {
    out.push_back(static_cast<uint8_t>(id >> shift));
  }
}

void append_size(Bytes& out, uint64_t value) {
  int bytes = 1;
  while (bytes < 8 && value >= ((uint64_t{1} << (bytes * 7)) - 1)) ++bytes;
  std::array<uint8_t, 8> encoded{};
  for (int i = bytes - 1; i >= 0; --i) {
    encoded[i] = static_cast<uint8_t>(value);
    value >>= 8;
  }
  encoded[0] |= static_cast<uint8_t>(1U << (8 - bytes));
  out.insert(out.end(), encoded.begin(), encoded.begin() + bytes);
}

void append_element(Bytes& out, uint32_t id, const uint8_t* data,
                    std::size_t size) {
  append_id(out, id);
  append_size(out, size);
  out.insert(out.end(), data, data + size);
}

void append_element(Bytes& out, uint32_t id, const Bytes& data) {
  append_element(out, id, data.data(), data.size());
}

void append_uint(Bytes& out, uint32_t id, uint64_t value) {
  int bytes = 1;
  while (bytes < 8 && value >= (uint64_t{1} << (bytes * 8))) ++bytes;
  std::array<uint8_t, 8> encoded{};
  for (int i = bytes - 1; i >= 0; --i) {
    encoded[i] = static_cast<uint8_t>(value);
    value >>= 8;
  }
  append_element(out, id, encoded.data(), bytes);
}

void append_string(Bytes& out, uint32_t id, const std::string& value) {
  append_element(out, id, reinterpret_cast<const uint8_t*>(value.data()),
                 value.size());
}

void write_bytes(std::ofstream& file, const Bytes& data) {
  file.write(reinterpret_cast<const char*>(data.data()),
             static_cast<std::streamsize>(data.size()));
}

void write_unknown_size_master(std::ofstream& file, uint32_t id) {
  Bytes encoded;
  append_id(encoded, id);
  encoded.push_back(0x01);
  encoded.insert(encoded.end(), 7, 0xFF);
  write_bytes(file, encoded);
}

class BitReader {
 public:
  BitReader(const uint8_t* data, std::size_t size) : m_data(data), m_size(size) {}

  bool read_bit(bool& bit) {
    if (m_bit >= m_size * 8) return false;
    bit = (m_data[m_bit / 8] & (0x80U >> (m_bit % 8))) != 0;
    ++m_bit;
    return true;
  }

  bool read_unsigned_exp_golomb(uint32_t& value) {
    unsigned leading_zeroes = 0;
    bool bit = false;
    while (read_bit(bit) && !bit) {
      if (++leading_zeroes > 31) return false;
    }
    if (!bit) return false;
    uint32_t suffix = 0;
    for (unsigned i = 0; i < leading_zeroes; ++i) {
      if (!read_bit(bit)) return false;
      suffix = (suffix << 1) | static_cast<uint32_t>(bit);
    }
    value = ((uint32_t{1} << leading_zeroes) - 1) + suffix;
    return true;
  }

 private:
  const uint8_t* m_data;
  std::size_t m_size;
  std::size_t m_bit = 0;
};

}  // namespace

MatroskaRecorder::~MatroskaRecorder() { close(); }

bool MatroskaRecorder::open(const std::string& path, bool h265, int width,
                            int height, int fps) {
  close();
  m_file.open(path, std::ios::binary | std::ios::out | std::ios::trunc);
  if (!m_file.is_open()) return false;
  m_h265 = h265;
  m_width = width;
  m_height = height;
  m_fps = std::max(1, fps);
  m_header_written = false;
  m_access_unit_has_vcl = false;
  m_access_unit_keyframe = false;
  m_frame_index = 0;
  m_cluster_timestamp_ms = 0;
  m_access_unit.clear();
  m_vps.clear();
  m_sps.clear();
  m_pps.clear();
  return true;
}

const uint8_t* MatroskaRecorder::strip_start_code(const uint8_t* data,
                                                  std::size_t& size) {
  if (size >= 4 && data[0] == 0 && data[1] == 0 && data[2] == 0 &&
      data[3] == 1) {
    data += 4;
    size -= 4;
  } else if (size >= 3 && data[0] == 0 && data[1] == 0 && data[2] == 1) {
    data += 3;
    size -= 3;
  }
  return data;
}

std::vector<uint8_t> MatroskaRecorder::remove_emulation_prevention(
    const uint8_t* data, std::size_t size) {
  std::vector<uint8_t> result;
  result.reserve(size);
  unsigned zeroes = 0;
  for (std::size_t i = 0; i < size; ++i) {
    if (zeroes >= 2 && data[i] == 3) {
      zeroes = 0;
      continue;
    }
    result.push_back(data[i]);
    zeroes = data[i] == 0 ? zeroes + 1 : 0;
  }
  return result;
}

bool MatroskaRecorder::h264_first_slice(const uint8_t* nalu,
                                        std::size_t size) {
  if (size <= 1) return true;
  const auto rbsp = remove_emulation_prevention(nalu + 1, size - 1);
  BitReader reader(rbsp.data(), rbsp.size());
  uint32_t first_mb = 0;
  return reader.read_unsigned_exp_golomb(first_mb) && first_mb == 0;
}

bool MatroskaRecorder::h265_first_slice(const uint8_t* nalu,
                                        std::size_t size) {
  return size > 2 && (nalu[2] & 0x80U) != 0;
}

void MatroskaRecorder::store_codec_config(int type, const uint8_t* nalu,
                                          std::size_t size) {
  if ((!m_h265 && type == 7) || (m_h265 && type == 33)) {
    m_sps.assign(nalu, nalu + size);
  } else if ((!m_h265 && type == 8) || (m_h265 && type == 34)) {
    m_pps.assign(nalu, nalu + size);
  } else if (m_h265 && type == 32) {
    m_vps.assign(nalu, nalu + size);
  }
}

bool MatroskaRecorder::codec_config_ready() const {
  return !m_sps.empty() && !m_pps.empty() && (!m_h265 || !m_vps.empty());
}

std::vector<uint8_t> MatroskaRecorder::make_codec_private() const {
  Bytes result;
  if (!m_h265) {
    if (m_sps.size() < 4 || m_sps.size() > 0xFFFF ||
        m_pps.size() > 0xFFFF) {
      return {};
    }
    result = {1, m_sps[1], m_sps[2], m_sps[3], 0xFF, 0xE1,
              static_cast<uint8_t>(m_sps.size() >> 8),
              static_cast<uint8_t>(m_sps.size())};
    result.insert(result.end(), m_sps.begin(), m_sps.end());
    result.push_back(1);
    result.push_back(static_cast<uint8_t>(m_pps.size() >> 8));
    result.push_back(static_cast<uint8_t>(m_pps.size()));
    result.insert(result.end(), m_pps.begin(), m_pps.end());
    return result;
  }

  if (m_sps.size() <= 2 || m_vps.size() > 0xFFFF ||
      m_sps.size() > 0xFFFF || m_pps.size() > 0xFFFF) {
    return {};
  }
  const auto rbsp = remove_emulation_prevention(m_sps.data() + 2,
                                                 m_sps.size() - 2);
  if (rbsp.size() < 13) return {};
  const uint8_t temporal_layers = ((rbsp[0] >> 1) & 0x07) + 1;
  const uint8_t temporal_nested = rbsp[0] & 0x01;
  result.push_back(1);
  result.push_back(rbsp[1]);
  result.insert(result.end(), rbsp.begin() + 2, rbsp.begin() + 13);
  result.insert(result.end(), {0xF0, 0x00, 0xFC, 0xFD, 0xF8, 0xF8, 0x00,
                               0x00});
  result.push_back(static_cast<uint8_t>((temporal_layers << 3) |
                                        (temporal_nested << 2) | 3));
  result.push_back(3);
  const auto append_array = [&result](uint8_t type, const Bytes& nalu) {
    result.push_back(static_cast<uint8_t>(0x80 | type));
    result.push_back(0);
    result.push_back(1);
    result.push_back(static_cast<uint8_t>(nalu.size() >> 8));
    result.push_back(static_cast<uint8_t>(nalu.size()));
    result.insert(result.end(), nalu.begin(), nalu.end());
  };
  append_array(32, m_vps);
  append_array(33, m_sps);
  append_array(34, m_pps);
  return result;
}

bool MatroskaRecorder::write_header() {
  if (m_header_written) return true;
  if (!codec_config_ready()) return false;
  const auto codec_private = make_codec_private();
  if (codec_private.empty()) return false;

  Bytes ebml;
  append_uint(ebml, 0x4286, 1);
  append_uint(ebml, 0x42F7, 1);
  append_uint(ebml, 0x42F2, 4);
  append_uint(ebml, 0x42F3, 8);
  append_string(ebml, 0x4282, "matroska");
  append_uint(ebml, 0x4287, 4);
  append_uint(ebml, 0x4285, 2);
  Bytes ebml_element;
  append_element(ebml_element, 0x1A45DFA3, ebml);
  write_bytes(m_file, ebml_element);

  write_unknown_size_master(m_file, 0x18538067);

  Bytes info;
  append_uint(info, 0x2AD7B1, 1000000);
  append_string(info, 0x4D80, "OpenHD native Matroska muxer");
  append_string(info, 0x5741, "OpenHD");
  Bytes info_element;
  append_element(info_element, 0x1549A966, info);
  write_bytes(m_file, info_element);

  Bytes video;
  append_uint(video, 0xB0, static_cast<uint64_t>(m_width));
  append_uint(video, 0xBA, static_cast<uint64_t>(m_height));
  Bytes video_element;
  append_element(video_element, 0xE0, video);

  Bytes track;
  append_uint(track, 0xD7, 1);
  append_uint(track, 0x73C5, 1);
  append_uint(track, 0x83, 1);
  append_uint(track, 0x9C, 0);
  append_uint(track, 0x23E383,
              1000000000ULL / static_cast<uint64_t>(m_fps));
  append_string(track, 0x86,
                m_h265 ? "V_MPEGH/ISO/HEVC" : "V_MPEG4/ISO/AVC");
  append_element(track, 0x63A2, codec_private);
  track.insert(track.end(), video_element.begin(), video_element.end());
  Bytes track_element;
  append_element(track_element, 0xAE, track);
  Bytes tracks;
  append_element(tracks, 0x1654AE6B, track_element);
  write_bytes(m_file, tracks);
  m_header_written = m_file.good();
  return m_header_written;
}

void MatroskaRecorder::append_nalu(const uint8_t* nalu, std::size_t size) {
  const uint32_t length = static_cast<uint32_t>(size);
  m_access_unit.push_back(static_cast<uint8_t>(length >> 24));
  m_access_unit.push_back(static_cast<uint8_t>(length >> 16));
  m_access_unit.push_back(static_cast<uint8_t>(length >> 8));
  m_access_unit.push_back(static_cast<uint8_t>(length));
  m_access_unit.insert(m_access_unit.end(), nalu, nalu + size);
}

void MatroskaRecorder::start_cluster(uint64_t timestamp_ms) {
  m_cluster_timestamp_ms = timestamp_ms;
  write_unknown_size_master(m_file, 0x1F43B675);
  Bytes timestamp;
  append_uint(timestamp, 0xE7, timestamp_ms);
  write_bytes(m_file, timestamp);
}

void MatroskaRecorder::write_access_unit() {
  if (!m_access_unit_has_vcl || m_access_unit.empty()) {
    m_access_unit.clear();
    m_access_unit_has_vcl = false;
    m_access_unit_keyframe = false;
    return;
  }
  if (!write_header()) return;

  const uint64_t timestamp_ms =
      m_frame_index * 1000ULL / static_cast<uint64_t>(m_fps);
  if (m_frame_index == 0 || timestamp_ms - m_cluster_timestamp_ms >= 1000) {
    start_cluster(timestamp_ms);
  }
  const int64_t relative = static_cast<int64_t>(timestamp_ms) -
                           static_cast<int64_t>(m_cluster_timestamp_ms);
  Bytes block;
  block.reserve(m_access_unit.size() + 4);
  block.push_back(0x81);
  block.push_back(static_cast<uint8_t>((relative >> 8) & 0xFF));
  block.push_back(static_cast<uint8_t>(relative & 0xFF));
  block.push_back(m_access_unit_keyframe ? 0x80 : 0x00);
  block.insert(block.end(), m_access_unit.begin(), m_access_unit.end());
  Bytes element;
  append_element(element, 0xA3, block);
  write_bytes(m_file, element);
  ++m_frame_index;
  m_access_unit.clear();
  m_access_unit_has_vcl = false;
  m_access_unit_keyframe = false;
}

void MatroskaRecorder::feed_nalu(const uint8_t* data, std::size_t size) {
  if (!m_file.is_open() || !data || size == 0) return;
  const uint8_t* nalu = strip_start_code(data, size);
  if (size == 0) return;
  const int type = m_h265 ? ((nalu[0] >> 1) & 0x3F) : (nalu[0] & 0x1F);
  const bool config = m_h265 ? (type >= 32 && type <= 34)
                             : (type == 7 || type == 8);
  const bool vcl = m_h265 ? type <= 31 : (type >= 1 && type <= 5);
  const bool aud = m_h265 ? type == 35 : type == 9;

  if (config) {
    if (m_access_unit_has_vcl) write_access_unit();
    store_codec_config(type, nalu, size);
    return;
  }
  if (aud) {
    if (m_access_unit_has_vcl) write_access_unit();
    return;
  }
  if (vcl) {
    const bool first_slice = m_h265 ? h265_first_slice(nalu, size)
                                    : h264_first_slice(nalu, size);
    if (first_slice && m_access_unit_has_vcl) write_access_unit();
    append_nalu(nalu, size);
    m_access_unit_has_vcl = true;
    m_access_unit_keyframe =
        m_access_unit_keyframe || (m_h265 ? (type >= 16 && type <= 23)
                                         : type == 5);
    return;
  }

  if (m_access_unit_has_vcl) write_access_unit();
  append_nalu(nalu, size);
}

void MatroskaRecorder::flush() {
  if (m_file.is_open()) m_file.flush();
}

void MatroskaRecorder::close() {
  if (!m_file.is_open()) return;
  write_access_unit();
  m_file.flush();
  m_file.close();
  m_access_unit.clear();
}
