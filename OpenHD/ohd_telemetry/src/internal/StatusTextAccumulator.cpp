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

void StatusTextAccumulator::processLogMessage(
    openhd::log::udp::LogMessage msg) {
  //std::cout<<"Log message:"<<msg.message<<"\n";
  assert(msg.hasNullTerminator());
  std::lock_guard<std::mutex> guard(bufferedLogMessagesLock);
  bufferedLogMessages.push(msg);
}

void StatusTextAccumulator::processLogMessageData(const uint8_t *data,
                                                  std::size_t dataLen) {
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
