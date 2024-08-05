// Rewritten by Rapha 08.24'

#include "openhd_led.h"

#include <chrono>
#include <thread>
#include <atomic>
#include <string>
#include <memory>

#include "openhd_platform.h"
#include "openhd_spdlog.h"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"

void toggle_led(const std::string& filename, const bool on) {
    const auto content = on ? "1" : "0";
    OHDFilesystemUtil::write_file(filename, content);
}

void led_on_off_delayed(const std::string& filename, const std::chrono::milliseconds& delay1, const std::chrono::milliseconds& delay2) {
    toggle_led(filename, false);
    std::this_thread::sleep_for(delay1);
    toggle_led(filename, true);
    std::this_thread::sleep_for(delay2);
}

void blink_leds_fast(const std::string& primary_filename, const std::string& secondary_filename, const std::chrono::milliseconds& delay) {
    led_on_off_delayed(secondary_filename, delay, delay);
    led_on_off_delayed(primary_filename, delay, delay);
}

void blink_leds_slow(const std::string& primary_filename, const std::string& secondary_filename, const std::chrono::milliseconds& delay) {
    led_on_off_delayed(primary_filename, delay, delay);
}

void blink_leds_alternating(const std::string& primary_filename, const std::string& secondary_filename, const std::chrono::milliseconds& delay, std::atomic<bool>& running) {
    while (running) {
        toggle_led(secondary_filename, true);
        toggle_led(primary_filename, false);
        std::this_thread::sleep_for(delay);
        toggle_led(secondary_filename, false);
        toggle_led(primary_filename, true);
        std::this_thread::sleep_for(delay);
    }
}

class LEDPaths {
public:
    std::string primary_led_filename;
    std::string secondary_led_filename;

    LEDPaths(const std::string& primary, const std::string& secondary) :
        primary_led_filename(primary), secondary_led_filename(secondary) {}
};

LEDPaths get_led_paths() {
    if (OHDPlatform::instance().is_rpi()) {
        return { "/sys/class/leds/ACT/brightness", "/sys/class/leds/PWR/brightness" };
    } else if (OHDPlatform::instance().is_zero3w()) {
        return { "/sys/class/leds/board-led/brightness", "/sys/class/leds/mmc0::/brightness" };
    } else if (OHDPlatform::instance().is_radxa_cm3()) {
        return { "/sys/class/leds/pi-led-green/brightness", "/sys/class/leds/pwr-led-red/brightness" };
    }
    return { "", "" };
}

class LEDManager {
public:
    static LEDManager& instance() {
        static LEDManager instance;
        return instance;
    }

    void set_secondary_led_status(int status) {
        const bool on = status != STATUS_ON;
        const auto paths = get_led_paths();
        toggle_led(paths.secondary_led_filename, on);
    }

    void set_primary_led_status(int status) {
        const bool on = status != STATUS_ON;
        const auto paths = get_led_paths();
        toggle_led(paths.primary_led_filename, on);
    }

    void start_loading_thread() {
        if (m_running) return;

        m_running = true;
        m_loading_thread = std::make_unique<std::thread>(&LEDManager::loading_loop, this);
    }

    void stop_loading_thread() {
        if (m_running) {
            m_running = false;
            if (m_loading_thread && m_loading_thread->joinable()) {
                m_loading_thread->join();
            }
            m_loading_thread = nullptr;
        }
    }

    void loading_loop() {
        while (m_running) {
            if (m_has_error) {
                blink_error();
            } else if (m_is_loading) {
                blink_loading();
            } else {
                blink_okay();
            }
        }
    }

    void blink_okay() {
        const auto paths = get_led_paths();
        blink_leds_fast(paths.primary_led_filename, paths.secondary_led_filename, std::chrono::milliseconds(50));
    }

    void blink_loading() {
        const auto paths = get_led_paths();
        blink_leds_slow(paths.primary_led_filename, paths.secondary_led_filename, std::chrono::milliseconds(200));
    }

    void blink_error() {
        const auto paths = get_led_paths();
        blink_leds_alternating(paths.primary_led_filename, paths.secondary_led_filename, std::chrono::milliseconds(50), m_running);
    }

    void set_status_okay() {
        if (m_is_loading) {
            stop_loading_thread();
        }
        m_has_error = false;
        m_is_loading = false;
        start_loading_thread();
    }

    void set_status_loading() {
        if (m_has_error) {
            stop_loading_thread();
        }
        m_has_error = false;
        m_is_loading = true;
        start_loading_thread();
    }

    void set_status_error() {
        if (m_is_loading) {
            stop_loading_thread();
        }
        m_has_error = true;
        m_is_loading = false;
        start_loading_thread();
    }

    void set_status_stopped() {
        stop_loading_thread();
        set_primary_led_status(STATUS_ON);
        set_secondary_led_status(STATUS_ON);
    }

private:
    LEDManager() : m_loading_thread(nullptr), m_running(false), m_has_error(false), m_is_loading(false) {}

    ~LEDManager() {
        stop_loading_thread();
    }

    std::unique_ptr<std::thread> m_loading_thread;
    std::atomic<bool> m_running;
    bool m_has_error;
    bool m_is_loading;
};
