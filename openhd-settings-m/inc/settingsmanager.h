
#include <openhd/mavlink.h>

#include <array>
#include <vector>

#include <boost/asio.hpp>

#include "openhd-microservice.hpp"
#include "openhd-platform.hpp"

#include "openhd-status.hpp"

#include "openhd-camera.hpp"



class SettingsManager: public Microservice {
public:

SettingsManager(boost::asio::io_service &io_service, PlatformType platform, bool is_air, std::string unit_id);
  
    void setup();
    void get_air_settings();
    void send_settings();
    void settings_listener();
    

 private:

    std::string m_unit_id;

    bool m_is_air = false;

    enum { max_length = 1024 };
    char data[max_length];

    //std::vector<StatusMessage> m_status_messages;

    boost::asio::ip::udp::socket m_udp_socket;

    std::string m_openhd_version = "unknown";

    std::vector<Camera> m_cameras;
};


