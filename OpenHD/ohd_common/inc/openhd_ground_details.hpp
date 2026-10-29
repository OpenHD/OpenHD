#pragma once
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <sys/statvfs.h>
#include <algorithm>
#include <cctype>
#include <array>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <sstream>
#include <string>

namespace openhd::ground_details {
using Payload = std::array<uint8_t, 76>;
inline std::string command(const std::string& value) {
  std::string result;
  if (auto* pipe = popen(value.c_str(), "r")) {
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe))
      if (result.size() < 16384) result += buffer;
    pclose(pipe);
  }
  return result;
}
inline void put64(Payload& data, unsigned offset, uint64_t value) {
  for (unsigned i=0;i<8;++i) data[offset+i]=value>>(8*i);
}
inline bool volume(Payload& data, unsigned offset, const std::string& path) {
  struct statvfs info{};
  if (statvfs(path.c_str(), &info) || !info.f_blocks) return false;
  put64(data,offset,uint64_t(info.f_blocks-info.f_bfree)*info.f_frsize);
  put64(data,offset+8,uint64_t(info.f_blocks)*info.f_frsize);
  return true;
}
inline bool ipv4(Payload& data, unsigned offset, std::string value) {
  while (!value.empty() && isspace(static_cast<unsigned char>(value.back()))) value.pop_back();
  in_addr address{};
  if (inet_pton(AF_INET,value.c_str(),&address)!=1) return false;
  std::memcpy(data.data()+offset,value.c_str(),std::min(size_t(15),value.size()));
  return true;
}
class Sampler {
 public:
  Payload sample() {
    Payload data{};std::memcpy(data.data(),"OHDG",4);data[4]=1;
    if(volume(data,12,"/")) data[5]|=1;
    const std::string iw = std::filesystem::exists("/usr/sbin/iw") ? "/usr/sbin/iw"
        : std::filesystem::exists("/sbin/iw") ? "/sbin/iw" : "iw";
    auto listing=command("timeout 2 "+iw+" dev 2>/dev/null");
    std::istringstream lines(listing);std::string line,iface,ap;
    while(std::getline(lines,line)) {
      std::istringstream words(line);std::string key,value;words>>key>>value;
      if(key=="Interface") iface=value;
      if(key=="type" && value=="AP") {ap=iface;break;}
    }
    if(!ap.empty() && ap.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.-")==std::string::npos) {
      auto stations=command("timeout 2 "+iw+" dev "+ap+" station dump 2>/dev/null; echo STATUS:$?");
      if(stations.find("STATUS:0")!=std::string::npos) {
        uint32_t count=0;std::istringstream rows(stations);
        while(std::getline(rows,line)) if(line.rfind("Station ",0)==0) ++count;
        for(unsigned i=0;i<4;++i)data[8+i]=count>>(8*i);
        data[5]|=4;
      }
      ifaddrs* addresses=nullptr;
      if(getifaddrs(&addresses)==0) {
        for(auto* a=addresses;a;a=a->ifa_next) if(a->ifa_addr && ap==a->ifa_name && a->ifa_addr->sa_family==AF_INET) {
          char value[INET_ADDRSTRLEN];auto* addr=reinterpret_cast<sockaddr_in*>(a->ifa_addr);
          if(inet_ntop(AF_INET,&addr->sin_addr,value,sizeof(value)) && ipv4(data,44,value)) {data[5]|=8;break;}
        }
        freeifaddrs(addresses);
      }
    }
    const auto now=std::chrono::steady_clock::now();
    if(last_public==std::chrono::steady_clock::time_point{} || now-last_public>=std::chrono::minutes(5)) {
      // Bounded HTTPS lookup of the public IPv4 egress address, never a LAN guess.
      public_ip=command("curl -4 --fail --silent --max-time 2 --connect-timeout 1 --max-filesize 64 https://api.ipify.org 2>/dev/null");
      last_public=now;
    }
    if(ipv4(data,60,public_ip))data[5]|=16;
    return data;
  }
 private:
  std::string public_ip;
  std::chrono::steady_clock::time_point last_public{};
};
}
