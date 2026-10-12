// SPDX-License-Identifier: GPL-3.0-only
// Transport-independent, bounded MAVLink FTP metadata implementation.
#pragma once
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace openhd::settings {
using FtpPayload = std::array<uint8_t, 251>;
inline uint32_t crc32(const std::string& bytes) {
  uint32_t crc = 0xffffffff;
  for (const unsigned char c : bytes) {
    crc ^= c;
    for (int i=0; i<8; ++i) crc = (crc >> 1) ^ (0xedb88320u & (0u - (crc & 1)));
  }
  return ~crc;
}
inline uint32_t u32(const uint8_t* p) {
  return uint32_t(p[0]) | (uint32_t(p[1])<<8) | (uint32_t(p[2])<<16) | (uint32_t(p[3])<<24);
}
inline void put32(uint8_t* p, uint32_t n) { for (int i=0;i<4;++i) p[i]=uint8_t(n>>(8*i)); }

class MetadataFtpServer {
 public:
  using Files = std::map<std::string, std::string>;
  Files files;
  std::optional<FtpPayload> handle(uint8_t sys, uint8_t comp, const FtpPayload& q) {
    if (q[3] >= 128 || q[4] > 239) return std::nullopt; // Not a request, or malformed.
    FtpPayload r{};
    const uint16_t seq = uint16_t(q[0] | (uint16_t(q[1])<<8)) + 1;
    r[0]=uint8_t(seq); r[1]=uint8_t(seq>>8); r[2]=q[2]; r[3]=128; r[5]=q[3];
    const auto key = std::make_pair(sys,comp);
    const auto fail = [&](uint8_t err) { r[3]=129; r[4]=1; r[12]=err; };
    const auto now = std::chrono::steady_clock::now();
    for (auto it=sessions.begin();it!=sessions.end();) {
      if (now-it->second.touched > std::chrono::seconds(30)) it=sessions.erase(it); else ++it;
    }
    if (q[3]==4 || q[3]==14) { // OpenFileRO / CalcFileCRC32
      if (q[4]==0) { fail(1); return r; }
      const auto end = std::find(q.begin()+12,q.begin()+12+q[4],0);
      const std::string path(q.begin()+12,end);
      auto file=files.find(path);
      if (file==files.end()) { fail(10); return r; }
      if (q[3]==14) { r[4]=4; put32(r.data()+12,crc32(file->second)); return r; }
      if (sessions.size()>=16 && !sessions.count(key)) { fail(5); return r; }
      // Duplicate open requests replace the same client's session with an immutable snapshot.
      auto& s=sessions[key]; s.content=file->second; s.touched=now;
      r[2]=0; r[4]=4; put32(r.data()+12,uint32_t(s.content.size()));
    } else if (q[3]==5) { // ReadFile
      auto it=sessions.find(key);
      if (it==sessions.end() || q[2]!=0) { fail(4); return r; }
      auto& s=it->second; s.touched=now;
      const auto offset=u32(q.data()+8);
      if (offset>=s.content.size()) { fail(6); return r; }
      const auto count=std::min<size_t>(q[4],s.content.size()-offset);
      r[4]=uint8_t(count); put32(r.data()+8,offset);
      std::copy_n(s.content.begin()+offset,count,r.begin()+12);
    } else if (q[3]==1 || q[3]==2) { sessions.erase(key); }
    else fail(7); // Unsupported write/delete/list operations never touch disk.
    return r;
  }
 private:
  struct Session { std::string content; std::chrono::steady_clock::time_point touched; };
  std::map<std::pair<uint8_t,uint8_t>,Session> sessions;
};

class MetadataFtpClient {
 public:
  static constexpr size_t max_size=1024*1024;
  using Clock=std::chrono::steady_clock;
  bool active=false;
  bool success=false;
  std::string content;
  void start(const std::string& path) {
    content.clear(); success=false; active=true; size=0; session=0; sequence+=2;
    request={}; request[3]=4;
    if (path.size()>238) { active=false; return; }
    std::copy(path.begin(),path.end(),request.begin()+12); next();
    request[4]=uint8_t(path.size()+1);
  }
  std::optional<FtpPayload> poll() {
    if (!active || (attempts && Clock::now()-sent < std::chrono::milliseconds(500))) return std::nullopt;
    if (++attempts>5) { active=false; return std::nullopt; }
    sent=Clock::now(); return request;
  }
  bool handle(const FtpPayload& r) {
    const auto response_seq=uint16_t(r[0]|(uint16_t(r[1])<<8));
    if (!active || response_seq!=uint16_t(sequence+1) || r[5]!=request[3]) return false;
    if (r[3]!=128 || r[4]>239) { active=false; return true; }
    if (request[3]==4) {
      if (r[4]!=4 || (size=u32(r.data()+12))>max_size) { active=false; return true; }
      session=r[2]; content.reserve(size);
    } else {
      if (r[2]!=session || u32(r.data()+8)!=content.size() || r[4]==0 || content.size()+r[4]>size) { active=false; return true; }
      content.append(reinterpret_cast<const char*>(r.data()+12),r[4]);
    }
    if (content.size()==size) { active=false; success=true; return true; }
    request={}; request[2]=session; request[3]=5; request[4]=uint8_t(std::min<size_t>(239,size-content.size()));
    put32(request.data()+8,uint32_t(content.size())); sequence+=2; next(); return true;
  }
 private:
  FtpPayload request{};
  uint16_t sequence=0;
  uint8_t session=0;
  uint32_t size=0;
  int attempts=0;
  Clock::time_point sent{};
  void next() { request[0]=uint8_t(sequence); request[1]=uint8_t(sequence>>8); attempts=0; }
};
}
