#ifndef OPENHD_MATROSKA_RECORDER_H
#define OPENHD_MATROSKA_RECORDER_H

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

// Minimal streaming Matroska muxer for the Annex-B H.264/H.265 output emitted
// by rpicam-vid. It writes unknown-sized Segment and Cluster elements so the
// file remains usable when recording is interrupted without a final trailer.
class MatroskaRecorder {
 public:
  MatroskaRecorder() = default;
  ~MatroskaRecorder();
  MatroskaRecorder(const MatroskaRecorder&) = delete;
  MatroskaRecorder& operator=(const MatroskaRecorder&) = delete;

  bool open(const std::string& path, bool h265, int width, int height, int fps);
  void feed_nalu(const uint8_t* data, std::size_t size);
  void flush();
  void close();

  [[nodiscard]] bool is_open() const { return m_file.is_open(); }
  [[nodiscard]] bool good() const { return m_file.good(); }

 private:
  static const uint8_t* strip_start_code(const uint8_t* data,
                                         std::size_t& size);
  static std::vector<uint8_t> remove_emulation_prevention(const uint8_t* data,
                                                           std::size_t size);
  static bool h264_first_slice(const uint8_t* nalu, std::size_t size);
  static bool h265_first_slice(const uint8_t* nalu, std::size_t size);

  void store_codec_config(int type, const uint8_t* nalu, std::size_t size);
  bool codec_config_ready() const;
  std::vector<uint8_t> make_codec_private() const;
  bool write_header();
  void append_nalu(const uint8_t* nalu, std::size_t size);
  void write_access_unit();
  void start_cluster(uint64_t timestamp_ms);

  std::ofstream m_file;
  bool m_h265 = false;
  int m_width = 0;
  int m_height = 0;
  int m_fps = 30;
  bool m_header_written = false;
  bool m_access_unit_has_vcl = false;
  bool m_access_unit_keyframe = false;
  uint64_t m_frame_index = 0;
  uint64_t m_cluster_timestamp_ms = 0;
  std::vector<uint8_t> m_access_unit;
  std::vector<uint8_t> m_vps;
  std::vector<uint8_t> m_sps;
  std::vector<uint8_t> m_pps;
};

#endif  // OPENHD_MATROSKA_RECORDER_H
