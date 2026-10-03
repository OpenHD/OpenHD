#include "esp_analyse.h"
#include <cassert>
#include <cstdio>
struct Channel {int frequency;};
int main(int argc,char** argv) {
  if(argc==2) {
    const auto now=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    assert(openhd::read_esp_analysis(argv[1],now));
    return 0;
  }
  const std::string path="/tmp/test-esp-advice.json";
  nlohmann::json j={{"schema","openhd.rf_advice"},{"version",1},{"connected",true},{"scanning",true},
    {"generated_unix_ms",100000},{"expires_after_ms",5000},
    {"discovery",{{"schema","openhd.rf_discovery"},{"version",1},{"expires_after_ms",3000},{"priority_frequencies_hz",{2432000000LL,5180000000LL,2432000000LL}}}},
    {"channels", {{{"frequency_hz",2432000000LL},{"age_seconds",0.1},{"sampled_busy_percent",25.5},{"reason_codes",nlohmann::json::array()}}}}};
  auto read=[&](int64_t now=100000) {std::ofstream(path)<<j;return openhd::read_esp_analysis(path,now);};
  auto a=read();assert(a && a->priority_mhz==std::vector<int>{2432});
  assert(a->channels[0].busy_centipercent==2550);
  std::vector<Channel> channels={{2412},{2452},{2432}};
  openhd::prioritize_esp_channels(channels,*a);
  assert(channels[0].frequency==2432 && channels[1].frequency==2412 && channels[2].frequency==2452);
  assert(read(103000) && read(103000)->priority_mhz.empty());
  assert(!read(105000));assert(!read(99999));
  j["channels"][0]["frequency_hz"]=6085000000LL;
  j["discovery"]["priority_frequencies_hz"]={6085000000LL};
  assert(read()->priority_mhz==std::vector<int>{6085});
  j["channels"][0]["frequency_hz"]=2432000000LL;
  j["discovery"]["priority_frequencies_hz"]={2432000000LL};
  j["connected"]=false;assert(!read());j["connected"]=true;
  j["scanning"]=false;assert(!read());j["scanning"]=true;
  j["channels"][0]["age_seconds"]=2.9;
  assert(read(100200) && read(100200)->priority_mhz.empty());
  assert(!read(102200));
  j["channels"][0]["age_seconds"]=0.1;
  j["channels"][0]["reason_codes"]={"ADC_CLIPPING"};assert(!read());
  j["channels"][0]["reason_codes"]={"STALE_OR_GAIN_UNSTABLE"};assert(!read());
  j["channels"][0]["reason_codes"]=nlohmann::json::array();
  j["channels"][0]["sampled_busy_percent"]=101;assert(!read());
  j["channels"][0]["sampled_busy_percent"]=25.5;j["version"]=2;assert(!read());
  std::ofstream(path)<<"broken";assert(!openhd::read_esp_analysis(path,100000));
  std::remove(path.c_str());
}
