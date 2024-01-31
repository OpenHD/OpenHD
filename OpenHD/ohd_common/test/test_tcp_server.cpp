//
// Created by consti10 on 31.01.24.
//

#include "openhd_tcp.h"


class TestServer:public openhd::TCPServer{
  explicit TestServer(): openhd::TCPServer("Test",openhd::TCPServer::Config()){};
  void on_external_device(std::string ip, bool connected)override{

  };
  void on_packet_any_tcp_client(const uint8_t* data, int data_len)override{

  };
};


int main(int argc, char *argv[]) {


}