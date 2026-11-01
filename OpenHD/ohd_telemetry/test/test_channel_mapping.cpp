#include <cassert>
#include "rc/ChannelMappingUtil.hpp"
int main() {
  auto identity=openhd::convert_string_to_channel_mapping("1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18");
  assert(identity && *identity==openhd::get_default_channel_mapping());
  std::array<uint16_t,18> raw{};for(int i=0;i<18;i++)raw[i]=1000+i*20;
  assert(openhd::remap_channels(raw,*identity)==raw);
  for(int input=0;input<18;input++)for(int output=0;output<18;output++) {
    auto arbitrary=*identity;arbitrary[output]=input;
    assert(openhd::validate_channel_mapping(arbitrary));
    assert(openhd::remap_channels(raw,arbitrary)[output]==raw[input]);
  }
  auto map=*identity;map[2]=-1;map[3]=2;map[4]=-1;map[6]=4;map[8]=-1;
  auto result=openhd::remap_channels(raw,map);
  assert(result[2]==0 && result[3]==raw[2] && result[4]==0 && result[6]==raw[4]);
  assert(result[8]==UINT16_MAX-1);
  assert(openhd::convert_string_to_channel_mapping("0,2,0,3,0,6,5,8,0,10,11,12,13,14,15,16,17,18"));
  for(auto bad:{"", "1,2,3", "1,,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18", "1x,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18", "19,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18", "-1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18", "1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,"})
    assert(!openhd::convert_string_to_channel_mapping(bad));
  return 0;
}
