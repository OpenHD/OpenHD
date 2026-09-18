#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "matroska_recorder.h"

namespace {

bool is_start_code(const std::vector<uint8_t>& data, std::size_t offset) {
  return offset + 2 < data.size() && data[offset] == 0 &&
         data[offset + 1] == 0 &&
         (data[offset + 2] == 1 ||
          (offset + 3 < data.size() && data[offset + 2] == 0 &&
           data[offset + 3] == 1));
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 7) {
    std::cerr << "Usage: test_matroska_recorder INPUT OUTPUT h264|h265 "
                 "WIDTH HEIGHT FPS\n";
    return 2;
  }
  std::ifstream input(argv[1], std::ios::binary);
  const std::vector<uint8_t> stream{std::istreambuf_iterator<char>(input),
                                    std::istreambuf_iterator<char>()};
  if (!input && stream.empty()) return 3;

  MatroskaRecorder recorder;
  if (!recorder.open(argv[2], std::string(argv[3]) == "h265",
                     std::stoi(argv[4]), std::stoi(argv[5]),
                     std::stoi(argv[6]))) {
    return 4;
  }
  std::size_t start = 0;
  while (start < stream.size() && !is_start_code(stream, start)) ++start;
  while (start < stream.size()) {
    std::size_t next = start + 3;
    while (next < stream.size() && !is_start_code(stream, next)) ++next;
    recorder.feed_nalu(stream.data() + start, next - start);
    start = next;
  }
  recorder.close();
  return recorder.good() ? 0 : 5;
}
