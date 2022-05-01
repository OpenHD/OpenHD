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

SerialEndpoint::SerialEndpoint(std::string TAG,std::string serial_port):
MEndpoint(TAG),
SERIAL_PORT(std::move(serial_port)),m_serial(io_service){
    std::cout<<"SerialEndpoint created "<<SERIAL_PORT<<"\n";
    mOpenSerialPortThread = std::make_unique<boost::thread>([this] { safeRestart(); });
}

void SerialEndpoint::safeCloseCleanup() {
    if(m_serial.is_open()){
        m_serial.close();
    }
}

void SerialEndpoint::safeRestart() {
    bool opened=false;
    while (!opened){
        try {
            std::cout<< "Opening serial port: " << SERIAL_PORT <<"\n";
            m_serial.open(SERIAL_PORT);
        } catch (boost::system::system_error::exception& e) {
            std::cerr <<"Failed to open serial port \n";
            std::this_thread::sleep_for(RECONNECT_DELAY);
            continue;
        }
        try {
            //m_serial.set_option(boost::asio::serial_port_base::baud_rate(BAUD));
            m_serial.set_option(boost::asio::serial_port_base::baud_rate(115200));
            /*m_serial.set_option(boost::asio::serial_port_base::character_size(8));
            m_serial.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));
            m_serial.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
            m_serial.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));*/
        } catch (boost::system::system_error::exception& e) {
            std::cerr <<"Faild to set serial port baud rate"<<BAUD<<"\n";
            m_serial.close();
            std::this_thread::sleep_for(RECONNECT_DELAY);
            continue;
        }
        std::cout<<"Opened Serial, ready to read or write data\n";
        opened= true;
        mIoThread=std::make_unique<boost::thread>(boost::bind(&boost::asio::io_service::run, &io_service));
    }
    startReceive();
}

void SerialEndpoint::startReceive() {
    //std::cout<<"SerialEndpoint::startReceive \n";
    m_serial.async_read_some(boost::asio::buffer(readBuffer.data(),readBuffer.size()),
                             boost::bind(&SerialEndpoint::handleRead,
                                         this,
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
}

void SerialEndpoint::handleRead(const boost::system::error_code& error,
                                size_t bytes_transferred) {
    if (!error) {
        //std::cout<<"SerialEndpoint::handleRead\n";
        MEndpoint::parseNewData(readBuffer.data(),bytes_transferred);
        startReceive();
    }else{
        std::cerr<<"SerialEndpoint::handleRead"<<error.message()<<"\n";
        safeCloseCleanup();
        safeRestart();
    }
}

void SerialEndpoint::handleWrite(const boost::system::error_code& error,
                                 size_t bytes_transferred) {
    if(error){
        std::cerr<<"SerialEndpoint::handleWrite "<<error.message()<<"\n";
        safeCloseCleanup();
        safeRestart();
    }
}

void SerialEndpoint::sendMessage(const MavlinkMessage &message) {
    if (!m_serial.is_open()) {
        std::cout << "SER: not open\n";
        return;
    }
    //std::cout<<"SerialEndpoint::sendMessage\n";
    const auto packed=message.pack();
    boost::asio::async_write(m_serial,
                             boost::asio::buffer(packed.data(),packed.size()),
                             boost::bind(&SerialEndpoint::handleWrite,
                                         this,
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
}



