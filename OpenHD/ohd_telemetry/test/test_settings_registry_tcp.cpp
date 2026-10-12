// Local integration fixture: exercise real MAVLink framing/provider with clients.
#include "mavsdk_temporary/XMavlinkParamProvider.h"
#include <arpa/inet.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <stdexcept>
const char* getVideoPath() { return "/tmp/"; }
int main() {
  XMavlinkParamProvider provider(101,100); int value=0;
  provider.add_param({"NEW_TEST",openhd::IntSetting{0,[&](std::string,int n){value=n;return true;},[&]{return value;}},
      []{return nlohmann::json{{"enforce",true},{"minimum",0},{"maximum",2}};}});
  provider.add_param(openhd::create_read_only_int("READ_ONLY",7));
  provider.add_param({"TEXT_TEST",openhd::StringSetting{"initial",[](std::string,std::string){return true;}}});
  provider.set_ready();
  const int server=socket(AF_INET,SOCK_STREAM,0); int reuse=1; setsockopt(server,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));
  sockaddr_in address{}; address.sin_family=AF_INET; address.sin_port=htons(15760); address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
  if(bind(server,reinterpret_cast<sockaddr*>(&address),sizeof(address)) || listen(server,4)) throw std::runtime_error("Fixture bind failed");
  std::cout<<"READY"<<std::endl;
  const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(60);
  int connections=0;
  while(connections<4 && std::chrono::steady_clock::now()<deadline) {
    pollfd listening{server,POLLIN,0}; if(poll(&listening,1,100)<=0) continue;
    const int client=accept(server,nullptr,nullptr); if(client<0) continue; ++connections;
    while(std::chrono::steady_clock::now()<deadline) {
      pollfd fd{client,POLLIN,0}; if(poll(&fd,1,100)<=0) continue;
      uint8_t buffer[4096]; const auto n=recv(client,buffer,sizeof(buffer),0); if(n<=0) break;
      std::vector<MavlinkMessage> messages;
      for(int i=0;i<n;++i) { MavlinkMessage message; mavlink_status_t status{};
        if(mavlink_parse_char(MAVLINK_COMM_0,buffer[i],&message.m,&status)) messages.push_back(message);
      }
      for(const auto& reply:provider.process_mavlink_messages(messages)) {
        uint8_t encoded[MAVLINK_MAX_PACKET_LEN]; const auto bytes=mavlink_msg_to_send_buffer(encoded,&reply.m);
        if(send(client,encoded,bytes,MSG_NOSIGNAL)!=bytes) throw std::runtime_error("Fixture send failed");
      }
    }
    close(client);
  }
  close(server);
}
