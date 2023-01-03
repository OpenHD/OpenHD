//
// Created by consti10 on 03.01.23.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_ETHERNET_LISTENER_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_ETHERNET_LISTENER_H_

class EthernetListener{
 public:
  EthernetListener();
  ~EthernetListener();
 private:
  std::unique_ptr<std::thread> loopThread;
  std::atomic<bool> loopThreadStop=false;
};

#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_ETHERNET_LISTENER_H_
