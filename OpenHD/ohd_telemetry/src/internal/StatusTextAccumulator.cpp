//
// Created by consti10 on 24.02.23.
//
#include "StatusTextAccumulator.h"

StatusTextAccumulator::StatusTextAccumulator() {
  m_console=openhd::log::create_or_get("l_acc");
  m_log_messages_receiver =
      std::make_unique<SocketHelper::UDPReceiver>(SocketHelper::ADDRESS_LOCALHOST,
                                                  openhd::LOCAL_LOG_MESSAGES_UDP_PORT,
                                                  [this](const uint8_t *payload,
                                                         const std::size_t payloadSize) {
                                                    this->processLogMessageData(payload, payloadSize);
                                                  });
  m_log_messages_receiver->runInBackground();
}

StatusTextAccumulator::~StatusTextAccumulator() {
  m_log_messages_receiver->stopBackground();
}

void StatusTextAccumulator::processLogMessage(openhd::log::udp::LogMessage msg) {
  //std::cout<<"Log message:"<<msg.message<<"\n";
  assert(msg.hasNullTerminator());
  std::lock_guard<std::mutex> guard(bufferedLogMessagesLock);
  bufferedLogMessages.push(msg);
}

void StatusTextAccumulator::processLogMessageData(const uint8_t *data,std::size_t dataLen) {
  //std::cout << "XX" << dataLen << "\n";
  //TODO this might discard messages
  if(dataLen != sizeof(openhd::log::udp::LogMessage)){
    m_console->warn("Invalid size for local log message {} wanted {}",dataLen,sizeof(openhd::log::udp::LogMessage));
    return ;
  }
  openhd::log::udp::LogMessage local_message{};
  memcpy((uint8_t *)&local_message, data, dataLen);
  const auto nullTerminatorFound = local_message.hasNullTerminator();
  if (!nullTerminatorFound) {
    m_console->warn("Log message without null terminator");
    return;
  }
  processLogMessage(local_message);
}

std::vector<openhd::log::udp::LogMessage>
StatusTextAccumulator::get_messages() {
  std::vector<openhd::log::udp::LogMessage> ret;
  std::lock_guard<std::mutex> guard(bufferedLogMessagesLock);
  while (!bufferedLogMessages.empty()) {
    const auto msg = bufferedLogMessages.front();
    if(!msg.hasNullTerminator()){
      m_console->warn("Somethings wrong with the log message");
    }else{
      ret.push_back(msg);
    }
    bufferedLogMessages.pop();
  }
  return ret;
}

static void convert(mavlink_message_t& mavlink_message,const openhd::log::udp::LogMessage& msg,uint8_t sys_id,uint8_t comp_id){
  mavlink_statustext_t mavlink_statustext;
  mavlink_statustext.id=0;
  mavlink_statustext.chunk_seq=0;
  mavlink_statustext.severity=msg.level;
  std::memcpy(&mavlink_statustext.text,msg.message,50);
  mavlink_msg_statustext_encode(sys_id,comp_id,&mavlink_message,&mavlink_statustext);
}

std::vector<MavlinkMessage> StatusTextAccumulator::get_mavlink_messages(uint8_t sys_id,uint8_t comp_id,int limit) {
  const auto messages= get_messages();
  std::vector<MavlinkMessage> ret;
  // limit to 5 to save bandwidth
  for(const auto& msg:messages){
    if (ret.size() < limit) {
      MavlinkMessage mavMsg;
      convert(mavMsg.m,msg,sys_id,comp_id);
      ret.push_back(mavMsg);
    } else {
      m_console->debug("Dropping log message {}",msg.msg_as_string());
    }
  }
  return ret;
}

void StatusTextAccumulator::manually_add_message(const std::string &text) {
  openhd::log::udp::LogMessage msg{};
  msg.level=static_cast<uint8_t>(openhd::log::udp::STATUS_LEVEL::DEBUG);
  std::strncpy((char *)msg.message,text.c_str(),50);
  processLogMessage(msg);
}
