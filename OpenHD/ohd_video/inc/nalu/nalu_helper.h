//
// Created by consti10 on 27.12.23.
//

#ifndef OPENHD_NALU_HELPER_H
#define OPENHD_NALU_HELPER_H

#include <unistd.h>

static  std::vector<std::shared_ptr<std::vector<uint8_t>>> make_fragments(const uint8_t* data,int data_len){
  std::vector<std::shared_ptr<std::vector<uint8_t>>> fragments;
  int bytes_used=0;
  const uint8_t* p=data;
  static constexpr auto MAX_FRAGMENT_SIZE=1024;
  while (true){
    const int remaining = (int)data_len-bytes_used;
    int len=0;
    if(remaining>MAX_FRAGMENT_SIZE){
      len = MAX_FRAGMENT_SIZE;
    }else{
      len = remaining;
    }
    std::shared_ptr<std::vector<uint8_t>> fragment=std::make_shared<std::vector<uint8_t>>(p,p+len);
    fragments.emplace_back(fragment);
    p = p+len;
    bytes_used += len;
    if(bytes_used==data_len){
      break ;
    }
  }
  return fragments;
}

static int find_next_nal(const uint8_t* data,int data_len){
  int nalu_search_state=0;
  for(int i=0;i<data_len;i++){
    switch (nalu_search_state) {
      case 0:
      case 1:
        if (data[i] == 0)
          nalu_search_state++;
        else
          nalu_search_state = 0;
        break;
      case 2:
      case 3:
        if(data[i]==0){
          nalu_search_state++;
        } else if(data[i]==1){
          // 0,0,0,1 or 0,0,1
          const int len=nalu_search_state==2 ? 2 : 3;
          if(i>len){
            return i-len;
          }
          nalu_search_state = 0;
        }else{
          nalu_search_state = 0;
        }
        break;
      default:
        break;
    }
  }
  return data_len;
}

#endif  // OPENHD_NALU_HELPER_H
