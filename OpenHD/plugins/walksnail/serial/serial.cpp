#include "serial.h"
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>


Serial::Serial(const std::string& port, int baudrate)
    : m_port(port), m_baud(baudrate), m_fd(-1) {}

Serial::~Serial()
{
    close();
}

bool Serial::open()
{
    m_fd = ::open(m_port.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (m_fd < 0)
        return false;

    termios tty{};
    tcgetattr(m_fd, &tty);

    speed_t spd = setBaudrate(m_baud);
    cfsetispeed(&tty, spd);
    cfsetospeed(&tty, spd);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_cflag &= ~(PARENB | CSTOPB | CRTSCTS);
    tty.c_cflag |=  (CREAD | CLOCAL);
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_iflag &= ~(IXON | IXOFF | IXANY | ICRNL);
    tty.c_oflag &= ~OPOST;
    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 10;

    if (tcsetattr(m_fd, TCSANOW, &tty) != 0) {
        ::close(m_fd);
        m_fd = -1;
        return false;
    }
    return true;
}

void Serial::close()
{
    if (m_fd >= 0) {
        ::close(m_fd);
        m_fd = -1;
    }
}

bool Serial::isOpen() const
{
    return m_fd >= 0;
}

int Serial::read(void* buf, size_t len)
{
    return ::read(m_fd, buf, len);
}

std::string Serial::readline()
{
    std::string line;
    char c;

    while (::read(m_fd, &c, 1) == 1) {
        if (c == '\n')
            break;
        if (c != '\r')
            line += c;
    }

    return line;
}

int Serial::write(const void* buf, size_t len)
{
    return ::write(m_fd, buf, len);
}

int Serial::write(const std::string& str)
{
    return write(str.c_str(), str.size());
}

speed_t Serial::setBaudrate(int baudrate)
{
    switch (baudrate) {
        case 9600:   return B9600;
        case 19200:  return B19200;
        case 38400:  return B38400;
        case 57600:  return B57600;
        case 115200: return B115200;
        default:     throw std::invalid_argument("Unsupported baud rate");
    }
}
