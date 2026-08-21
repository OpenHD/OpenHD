#include <arpa/inet.h>
#include <dlfcn.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cassert>
#include <cstring>
#include <iostream>
#include <string>

#include "openhd_plugin.h"

namespace {

void log_message(int32_t, const char*, const char*) {}

std::string receive_packet(int socket) {
  char buffer[128]{};
  const auto count = recv(socket, buffer, sizeof(buffer), 0);
  assert(count > 0);
  return {buffer, static_cast<std::size_t>(count)};
}

}  // namespace

int main(int argc, char** argv) {
  assert(argc == 2);
  int receiver = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
  assert(receiver >= 0);
  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  address.sin_port = 0;
  assert(bind(receiver, reinterpret_cast<const sockaddr*>(&address),
              sizeof(address)) == 0);
  socklen_t address_size = sizeof(address);
  assert(getsockname(receiver, reinterpret_cast<sockaddr*>(&address),
                     &address_size) == 0);
  const std::string port = std::to_string(ntohs(address.sin_port));
  assert(setenv("OPENHD_TOPOTEK_IP", "127.0.0.1", 1) == 0);
  assert(setenv("OPENHD_TOPOTEK_PORT", port.c_str(), 1) == 0);
  assert(setenv("OPENHD_TOPOTEK_CLIENT_PORT", "19004", 1) == 0);

  void* library = dlopen(argv[1], RTLD_NOW | RTLD_LOCAL);
  assert(library != nullptr);
  auto get_descriptor = reinterpret_cast<openhd_plugin_get_descriptor_fn>(
      dlsym(library, OPENHD_PLUGIN_ENTRYPOINT));
  assert(get_descriptor != nullptr);
  const auto* descriptor = get_descriptor();
  assert(descriptor != nullptr);
  assert(std::strcmp(descriptor->name, "topotek") == 0);
  assert(descriptor->on_camera_control != nullptr);

  const openhd_plugin_host host{sizeof(host), OPENHD_PLUGIN_ABI_VERSION,
                                log_message};
  const openhd_plugin_context context{sizeof(context),
                                      OPENHD_PLUGIN_ROLE_AIR};
  assert(descriptor->init(&host, &context) == 0);

  const openhd_plugin_video_settings_event settings{
      sizeof(settings), 0, 3, 4096, OPENHD_PLUGIN_VIDEO_H264,
      1920, 1080, 30, "127.0.0.1"};
  descriptor->on_video_settings_changed(&settings);
  assert(receive_packet(receiver) == "#TPPD2wVID214A");
  assert(receive_packet(receiver) == "#TPPD2wBIT0346");

  const openhd_plugin_video_bitrate_event high_bitrate{
      sizeof(high_bitrate), 0, 3, 11000, OPENHD_PLUGIN_VIDEO_H264,
      1920, 1080, "127.0.0.1"};
  descriptor->on_video_bitrate_changed(&high_bitrate);
  assert(receive_packet(receiver) == "#TPPD2wBIT074A");

  const openhd_plugin_camera_control_event center{
      sizeof(center), 0, OPENHD_PLUGIN_GIMBAL_CENTER, 0.0F, 0.0F, 0};
  assert(descriptor->on_camera_control(&center) == 0);
  assert(receive_packet(receiver) == "#TPPG2wPTZ056A");

  const openhd_plugin_camera_control_event calibrate{
      sizeof(calibrate), 0, OPENHD_PLUGIN_GIMBAL_CALIBRATE, 0.0F, 0.0F, 0};
  assert(descriptor->on_camera_control(&calibrate) == 0);
  assert(receive_packet(receiver) == "#TPPG2wPTZ096E");

  const openhd_plugin_camera_control_event zoom{
      sizeof(zoom), 0, OPENHD_PLUGIN_CAMERA_ZOOM_RATE, 1.0F, 0.0F, 0};
  assert(descriptor->on_camera_control(&zoom) == 0);
  assert(receive_packet(receiver) == "#TPPM2wZMC0259");

  const openhd_plugin_camera_control_event roll{
      sizeof(roll), 0, OPENHD_PLUGIN_GIMBAL_ROLL_RATE, -0.45F, 0.0F, 0};
  assert(descriptor->on_camera_control(&roll) == 0);
  assert(receive_packet(receiver) == "#TPPG2wGSRD36A");

  const openhd_plugin_camera_control_event combined_view{
      sizeof(combined_view), 0, OPENHD_PLUGIN_CAMERA_IMAGE_TYPE, 2.0F, 0.0F, 0};
  assert(descriptor->on_camera_control(&combined_view) == 0);
  assert(receive_packet(receiver) == "#TPPD2wPIP104E");

  descriptor->shutdown();
  dlclose(library);
  close(receiver);
  std::cout << "Topotek plugin ABI and UDP packets passed\n";
  return 0;
}
