#pragma once
#include <string>
#include <termios.h>


class Serial {
public:
    Serial(const std::string& port, int baudrate = 9600);
    ~Serial();

    bool        open();
    void        close();
    bool        isOpen() const;

    int         read(void* buf, size_t len);
    std::string readline();
    int         write(const void* buf, size_t len);
    int         write(const std::string& str);

private:
    std::string m_port;
    int         m_baud;
    int         m_fd;

    speed_t     setBaudrate(int baudrate);
};
