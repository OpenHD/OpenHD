#include "fleet_video_lease.h"
#include <cassert>
#include <thread>

int main() {
  const int server = socket(AF_INET, SOCK_DGRAM, 0);
  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  assert(bind(server, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0);
  socklen_t size = sizeof(address);
  assert(getsockname(server, reinterpret_cast<sockaddr*>(&address), &size) == 0);
  timeval timeout{2, 0};
  setsockopt(server, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
  openhd::FleetVideoLease lease("127.0.0.1", ntohs(address.sin_port), true);
  assert(!lease.allowed());
  std::array<char, 16> packet{};
  sockaddr_in client{};
  assert(recvfrom(server, packet.data(), packet.size(), 0, reinterpret_cast<sockaddr*>(&client), &size) == 16);
  std::memcpy(packet.data(), "OHDFV1Y\0", 8);
  packet[15] ^= 1;
  sendto(server, packet.data(), packet.size(), 0, reinterpret_cast<sockaddr*>(&client), size);
  assert(!lease.allowed()); // A different request's reply cannot grant permission.
  packet[15] ^= 1;
  sendto(server, packet.data(), packet.size(), 0, reinterpret_cast<sockaddr*>(&client), size);
  assert(lease.allowed());
  std::this_thread::sleep_for(std::chrono::milliseconds(2100));
  assert(!lease.allowed()); // Losing the server stops Ground, rather than failing open.
  assert(recvfrom(server, packet.data(), packet.size(), 0, reinterpret_cast<sockaddr*>(&client), &size) == 16);
  std::memcpy(packet.data(), "OHDFV1N\0", 8);
  sendto(server, packet.data(), packet.size(), 0, reinterpret_cast<sockaddr*>(&client), size);
  assert(!lease.allowed());
  openhd::FleetVideoLease air("127.0.0.1", ntohs(address.sin_port), false);
  assert(air.allowed());
  close(server);
}
