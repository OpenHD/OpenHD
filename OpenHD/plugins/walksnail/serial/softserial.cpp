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
    if (m_open) return true;

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
    if (!m_open || len == 0) return -1;

    // Drain any prior transmission before touching wave resources
    while (gpioWaveTxBusy())
        gpioDelay(100);

    gpioWaveClear();

    int rc = gpioWaveAddSerial(
        m_gpio_tx, m_baud, 8, 2, 0, len,
        const_cast<char*>(static_cast<const char*>(buf)));

    if (rc < 0) return -1;   // wave pool still empty — don't call Create

    int wid = gpioWaveCreate();
    if (wid < 0) return -1;

    if (gpioWaveTxSend(wid, PI_WAVE_MODE_ONE_SHOT) < 0) {
        gpioWaveDelete(wid);
        return -1;
    }

    // Small delay so DMA actually starts before we poll busy
    gpioDelay(100);

    while (gpioWaveTxBusy())
        gpioDelay(100);

    gpioWaveDelete(wid);
    return static_cast<int>(len);
}

int SoftSerial::write(const std::string& str)
{
    return write(str.c_str(), str.size());
}
