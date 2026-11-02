#include "video_worker_log.h"
#include "video_worker_ipc.h"
#include <spdlog/spdlog.h>
#include <cassert>
#include <thread>
#include <vector>

int main() {
  using namespace openhd::video_worker;
  auto sink = std::make_shared<DiagnosticSink>();
  spdlog::logger logger("mpp_stream", sink);
  logger.set_level(spdlog::level::debug);
  logger.debug("ignored");
  logger.info("dual_encoder_active=true record_target_kbit_s=40000");
  DiagnosticSink::Entry entry;
  assert(sink->pop(entry));
  assert(entry.level == spdlog::level::info);
  assert(entry.text.find("mpp_stream: dual_encoder_active=true") == 0);
  int sockets[2];
  assert(socketpair(AF_UNIX, SOCK_SEQPACKET, 0, sockets) == 0);
  Header header{Type::Diagnostic};
  header.flags = entry.level;
  assert(send_packet(sockets[0], header, entry.text.data(), entry.text.size()));
  std::vector<uint8_t> payload;
  assert(receive_packet(sockets[1], header, payload) == 1);
  assert(header.type == Type::Diagnostic && header.flags == entry.level);
  assert(std::string(payload.begin(), payload.end()) == entry.text);
  Control automatic;
  automatic.bitrate = 20000;
  automatic.capacity_limit = true;
  automatic.automatic_allocation = true;
  assert(send_packet(sockets[0], {Type::Control}, &automatic, sizeof(automatic)));
  assert(receive_packet(sockets[1], header, payload) == 1);
  assert(header.type == Type::Control && payload.size() == sizeof(Control));
  Control received;
  std::memcpy(&received, payload.data(), sizeof(received));
  assert(received.bitrate == 20000 && received.capacity_limit &&
         received.automatic_allocation);
  close(sockets[0]); close(sockets[1]);
  assert(!sink->pop(entry));
  logger.warn("{}", std::string(4000, 'x'));
  assert(sink->pop(entry) && entry.text.size() == 2048);
  std::vector<std::thread> producers;
  for (int i = 0; i < 4; ++i)
    producers.emplace_back([&] { for (int j = 0; j < 100; ++j) logger.info("frame {}", j); });
  for (auto& producer : producers) producer.join();
  int count = 0;
  while (sink->pop(entry)) ++count;
  assert(count == 128);
  assert(sink->take_lost() == 272);
  assert(sink->take_lost() == 0);
  sink->dropped();
  assert(sink->take_lost() == 1);
}
