#include "softserial.h"
#include <pigpio.h>
#include <chrono>


SoftSerial::SoftSerial(uint8_t gpio_tx, uint8_t gpio_rx, int baudrate)
    : m_gpio_tx(gpio_tx), m_gpio_rx(gpio_rx), m_baud(baudrate), m_open(false)
{}

SoftSerial::~SoftSerial()
{
    close();
}

bool SoftSerial::open()
{
    if (gpioInitialise() < 0)
        return false;

    gpioSetMode(m_gpio_tx, PI_OUTPUT);
    gpioWrite(m_gpio_tx, 1);  // idle high

    if (gpioSerialReadOpen(m_gpio_rx, m_baud, 8) != 0) {
        gpioTerminate();
        return false;
    }

    m_open = true;
    return true;
}

void SoftSerial::close()
{
    if (!m_open) return;
    gpioSerialReadClose(m_gpio_rx);
    gpioTerminate();
    m_open = false;
}

bool SoftSerial::isOpen() const
{
    return m_open;
}

int SoftSerial::read(void* buf, size_t len)
{
    if (!m_open) return -1;
    int n = gpioSerialRead(m_gpio_rx, buf, len);
    return (n < 0) ? -1 : n;
}

std::string SoftSerial::readline(int timeout_ms)
{
    if (!m_open) return {};

    std::string line;
    char c;

    auto deadline = std::chrono::steady_clock::now()
                  + std::chrono::milliseconds(timeout_ms);

    while (std::chrono::steady_clock::now() < deadline) {
        int n = gpioSerialRead(m_gpio_rx, &c, 1);
        if (n == 1) {
            if (c == '\n') break;
            if (c != '\r') line += c;
        } else {
            gpioDelay(100);  // 100 µs poll interval
        }
    }

    return line;
}

int SoftSerial::write(const void* buf, size_t len)
{
    if (!m_open) return -1;

    // gpioWaveClear removes all existing waveforms — do not call this
    // from multiple threads or alongside other wave users.
    gpioWaveClear();

    // stophalfbits=2 means 1 stop bit (unit is half-bits)
    // pigpio's API takes char* but does not modify the buffer — cast is safe
    gpioWaveAddSerial(m_gpio_tx, m_baud, 8, 2, 0, len,
                      const_cast<char*>(static_cast<const char*>(buf)));

    int wid = gpioWaveCreate();
    if (wid < 0) return -1;

    gpioWaveTxSend(wid, PI_WAVE_MODE_ONE_SHOT);
    while (gpioWaveTxBusy())
        gpioDelay(100);

    gpioWaveDelete(wid);
    return static_cast<int>(len);
}

int SoftSerial::write(const std::string& str)
{
    return write(str.c_str(), str.size());
}
