#pragma once
#include <string>


class SoftSerial {
public:
    SoftSerial(const uint8_t gpio_tx, uint8_t gpio_rx, int baudrate = 9600);
    ~SoftSerial();

    bool        open();
    void        close();
    bool        isOpen() const;

    int         read(void* buf, size_t len);
    std::string readline(int timeout_ms);
    int         write(const void* buf, size_t len);
    int         write(const std::string& str);

private:
    uint8_t     m_gpio_tx;
    uint8_t     m_gpio_rx;
    int         m_baud;
    bool        m_open;
};
