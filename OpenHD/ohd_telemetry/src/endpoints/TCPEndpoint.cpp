//
// Created by consti10 on 13.04.22.
//

#include "TCPEndpoint.h"
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>
#include <boost/thread.hpp>
#include <utility>

#include "../mav_include.h"

TCPEndpoint::TCPEndpoint(std::string TAG, int Port) :
	MEndpoint(TAG),
	PORT(Port), _socket(_io_service) {
  allowConnectionThread = boost::thread(&TCPEndpoint::loopAllowConnection, this);
  std::cout << "TCPEndpoint created Port:" << PORT << "\n";
};

void TCPEndpoint::loopAllowConnection() {
  //while (true){
  //listen for new connection
  boost::asio::ip::tcp::acceptor
	  acceptor_(_io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), PORT));
  //waiting for connection
  std::cout << "Waiting for client to connect\n";
  acceptor_.accept(_socket);
  std::cout << "Connected\n";
  startReceive();
  //}
}

void TCPEndpoint::sendMessage(const MavlinkMessage &message) {
  debugMavlinkMessage(message.m, "TCPEndpoint::send");
  try {
	if (!_socket.is_open()) {
	  std::cout << "Socket not open\n";
	  return;
	}
	const auto tmp = message.pack();
	_socket.async_write_some(boost::asio::buffer(tmp.data(), tmp.size()),
							 [this](const boost::system::error_code &error, size_t bytes_transferred) {
							   std::cout << "TCP socket write error\n";
							 });
  } catch (const std::exception &e) {
	std::cerr << "TCP: catch handle_write error" << e.what() << std::endl;
  }
  //_socket.write_some(boost::asio::buffer(message.data(),message.data_len()));
}

void TCPEndpoint::startReceive() {
  std::cout << "start receive\n";
  _socket.async_read_some(boost::asio::buffer(readBuffer.data(), readBuffer.size()),
						  boost::bind(&TCPEndpoint::handleRead,
									  this,
									  boost::asio::placeholders::error,
									  boost::asio::placeholders::bytes_transferred));
}

void TCPEndpoint::handleRead(const boost::system::error_code &error, size_t bytes_transferred) {
  if (!error) {
	MEndpoint::parseNewData(readBuffer.data(), bytes_transferred);
	startReceive();
  } else {
	std::cerr << "SerialEndpoint::handleRead" << error.message() << "\n";
  }
}








