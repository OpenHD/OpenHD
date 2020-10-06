#include <fstream>

#include <iostream>
#include <iterator>
#include <exception>


#include <boost/asio.hpp>
#include <boost/bind.hpp>


#include "wifi.h"
#include "ethernet.h"


#include "json.hpp"


int main(int argc, char *argv[]) {

    boost::asio::io_service io_service;

    WiFi wifi(io_service);
    Ethernet ethernet(io_service);

    try {
        wifi.configure();
        ethernet.configure();

    } catch (std::exception &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        exit(1);
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        exit(1);
    }

    // fake it for the moment so the service doesn't exit, won't be needed once the microservice channel is wired in
    boost::asio::io_service::work work(io_service);

    io_service.run();

    return 0;
}
