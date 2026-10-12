#include "metadata_ftp.h"
#include <stdexcept>
#include <iostream>
using namespace openhd::settings;
void require(bool ok) { if (!ok) throw std::runtime_error("Metadata FTP regression"); }
int main() {
  require(crc32("123456789")==0xcbf43926);
  MetadataFtpServer server;
  const std::string expected(3000,'a'); server.files["/openhd/settings.json"]=expected;
  MetadataFtpClient client; client.start("/openhd/settings.json");
  int blocks=0;
  while (client.active) {
    auto request=client.poll(); require(bool(request));
    auto reply=server.handle(255,190,*request); require(bool(reply));
    // In-flight retries yield identical responses and do not advance an offset.
    require(server.handle(255,190,*request)==reply);
    require(client.handle(*reply));
    if (++blocks==2) server.files["/openhd/settings.json"]="changed";
  }
  require(client.success && client.content==expected); // Snapshot stays immutable.
  FtpPayload read{}; read[3]=5; read[4]=239;
  require(server.handle(254,190,read)->at(12)==4); // Other client cannot use session.
  FtpPayload write{}; write[3]=7;
  require(server.handle(255,190,write)->at(12)==7);
  client.start("/missing"); auto q=client.poll(); auto r=server.handle(255,190,*q);
  require(client.handle(*r) && !client.active && !client.success);
  client.start("/openhd/settings.json"); q=client.poll(); r=server.handle(255,190,*q);
  put32(r->data()+12,MetadataFtpClient::max_size+1);
  require(client.handle(*r) && !client.active && !client.success);
  std::cout << "METADATA FTP PASSED\n";
}
