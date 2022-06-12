//
// Created by consti10 on 13.04.22.
//

#include "SerialEndpoint.h"

#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>
#include <boost/thread.hpp>
#include <utility>

#include "../mav_include.h"

SerialEndpoint::SerialEndpoint(std::string TAG, HWOptions options,bool enableDebug) :
	MEndpoint(std::move(TAG)),
	m_options(std::move(options)),
	m_serial(io_service),
	m_enable_debug(enableDebug){
  mOpenSerialPortThread = std::make_unique<boost::thread>([this] { safeRestart(); });
  std::cout << "SerialEndpoint created " << m_options.linux_filename <<":"<<m_options.baud_rate<< "\n";
}

void SerialEndpoint::safeCloseCleanup() {
  if (m_serial.is_open()) {
	m_serial.close();
  }
}

void SerialEndpoint::safeRestart() {
  bool opened = false;
  while (!opened) {
	try {
	  if(m_enable_debug){
		std::cout <<TAG<< " opening serial port: " << m_options.linux_filename << "\n";
	  }
	  m_serial.open(m_options.linux_filename);
	} catch (boost::system::system_error::exception &e) {
	  // since we check every second, if there is no UART connected, we will get this error once a second.
	  // So only print it if we have debug enabled.
	  if(m_enable_debug){
		std::cerr <<TAG<<"Failed to open serial port \n";
	  }
	  std::this_thread::sleep_for(RECONNECT_DELAY);
	  continue;
	}
	try {
	  if (m_options.baud_rate != 0) {
		std::cout << "Setting baud rate to:" << m_options.baud_rate << "\n";
		m_serial.set_option(boost::asio::serial_port_base::baud_rate(m_options.baud_rate));
	  }
	  /*m_serial.set_option(boost::asio::serial_port_base::character_size(8));
	  m_serial.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));
	  m_serial.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
	  m_serial.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));*/
	} catch (boost::system::system_error::exception &e) {
	  std::cerr << "Faild to set serial port baud rate" << m_options.baud_rate << "\n";
	  m_serial.close();
	  std::this_thread::sleep_for(RECONNECT_DELAY);
	  continue;
	}
	std::cout << "Opened Serial, ready to read or write data\n";
	opened = true;
	mIoThread = std::make_unique<boost::thread>(boost::bind(&boost::asio::io_service::run, &io_service));
  }
  // some implementations need a heartbeat before they start sending data.
  auto msg = MExampleMessage::heartbeat();
  sendMessage(msg);
  startReceive();
}

void SerialEndpoint::startReceive() {
  //std::cout<<"SerialEndpoint::startReceive \n";
  m_serial.async_read_some(boost::asio::buffer(readBuffer.data(), readBuffer.size()),
						   boost::bind(&SerialEndpoint::handleRead,
									   this,
									   boost::asio::placeholders::error,
									   boost::asio::placeholders::bytes_transferred));
}

void SerialEndpoint::handleRead(const boost::system::error_code &error,
								size_t bytes_transferred) {
  if (!error) {
	//std::cout<<"SerialEndpoint::handleRead\n";
	MEndpoint::parseNewData(readBuffer.data(), (int)bytes_transferred);
	startReceive();
  } else {
	std::cerr << "SerialEndpoint::handleRead" << error.message() << "\n";
	safeCloseCleanup();
	safeRestart();
  }
}

void SerialEndpoint::handleWrite(const boost::system::error_code &error,
								 size_t bytes_transferred) {
  if (error) {
	std::cerr << "SerialEndpoint::handleWrite " << error.message() << "\n";
	safeCloseCleanup();
	safeRestart();
  }
}

void SerialEndpoint::sendMessageImpl(const MavlinkMessage &message) {
  if (!m_serial.is_open()) {
	std::cout << "SER: not open\n";
	return;
  }
  //std::cout<<"SerialEndpoint::sendMessage\n";
  const auto packed = message.pack();
  boost::asio::async_write(m_serial,
						   boost::asio::buffer(packed.data(), packed.size()),
						   boost::bind(&SerialEndpoint::handleWrite,
									   this,
									   boost::asio::placeholders::error,
									   boost::asio::placeholders::bytes_transferred));
}



