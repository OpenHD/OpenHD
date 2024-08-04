// Created by consti10 on 01.02.24.
//

#include "openhd_led.h"

#include <chrono>
#include <thread>
#include <utility>
#include <vector>
#include <string>
#include <algorithm>
#include <atomic>
#include <mutex>

#include "openhd_platform.h"
#include "openhd_spdlog.h"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"

std::vector<std::string> listLedFoldersWithBrightness(const std::string& baseDir) {
    std::vector<std::string> ledFolders;
    auto entries = OHDFilesystemUtil::getAllEntriesFullPathInDirectory(baseDir);

    for (const auto& entry : entries) {
        std::string brightnessFile = entry + "/brightness";
        if (OHDFilesystemUtil::exists(brightnessFile)) {
            std::string folderName = entry.substr(baseDir.length());
            if (folderName.front() == '/') folderName = folderName.substr(1);
            ledFolders.push_back(folderName);
        }
    }

    return ledFolders;
}

void setLedBrightness(const std::vector<std::string>& ledFolders, const std::string& baseDir, const std::string& value) {
    for (const auto& folder : ledFolders) {
        std::string brightnessFile = baseDir + folder + "/brightness";
        OHDFilesystemUtil::write_file(brightnessFile, value);
    }
}

void turnOffAllLeds(const std::vector<std::string>& ledFolders, const std::string& baseDir) {
    setLedBrightness(ledFolders, baseDir, "0");
}

void turnOnAllLeds(const std::vector<std::string>& ledFolders, const std::string& baseDir) {
    setLedBrightness(ledFolders, baseDir, "1");
}

namespace openhd::rpi {
    void toggle_led(const std::string& filename, bool on) {
        if (!OHDFilesystemUtil::exists(filename)) {
            return;
        }
        OHDFilesystemUtil::write_file(filename, on ? "1" : "0");
    }

    void toggle_secondary_led(bool on) {
        static constexpr auto filename = "/sys/class/leds/PWR/brightness";
        toggle_led(filename, on);
    }

    void toggle_primary_led(bool on) {
        static constexpr auto filename = "/sys/class/leds/ACT/brightness";
        toggle_led(filename, on);
    }

    void toggle_led_delayed(bool on, const std::chrono::milliseconds& delay) {
        toggle_secondary_led(on);
        std::this_thread::sleep_for(delay);
        toggle_secondary_led(!on);
    }
}

namespace openhd::zero3w {
    void toggle_led(const std::string& filename, bool on) {
        OHDFilesystemUtil::write_file(filename, on ? "1" : "0");
    }

    void toggle_secondary_led(bool on) {
        static constexpr auto filename = "/sys/class/leds/mmc0::/brightness";
        toggle_led(filename, on);
    }

    void toggle_primary_led(bool on) {
        static constexpr auto filename = "/sys/class/leds/board-led/brightness";
        toggle_led(filename, on);
    }
}

namespace openhd::radxacm3 {
    void toggle_led(const std::string& filename, bool on) {
        OHDFilesystemUtil::write_file(filename, on ? "1" : "0");
    }

    void toggle_secondary_led(bool on) {
        static constexpr auto filename = "/sys/class/leds/pwr-led-red/brightness";
        toggle_led(filename, on);
    }

    void toggle_primary_led(bool on) {
        static constexpr auto filename = "/sys/class/leds/pi-led-green/brightness";
        toggle_led(filename, on);
    }
}

namespace openhd {
    class LEDManager {
    public:
        static LEDManager& instance();

        void set_led_status(bool on, const std::function<void(bool)>& toggle_led_fn);
        void set_aux_led_status(int status);
        void set_rgb_led_status(int status, int color);
        void set_secondary_led_status(int status);
        void set_primary_led_status(int status);
        void set_status_okay();
        void set_status_loading();
        void set_status_error();
        
        void start_blinking_primary_led(int frequency_hz, int duration_seconds);
        void stop_blinking_primary_led();

    private:
        LEDManager();
        ~LEDManager();

        void loop();
        void blink_leds(int frequency_hz, int duration_seconds);

        std::atomic<bool> m_blinking{false};
        std::thread m_blinking_thread;
        std::mutex m_blinking_mutex;
        bool m_blinking_running{false};

        void blink_leds_thread(int frequency_hz, int duration_seconds);
    };

    LEDManager& LEDManager::instance() {
        static LEDManager instance{};
        return instance;
    }

    void LEDManager::set_led_status(bool on, const std::function<void(bool)>& toggle_led_fn) {
        toggle_led_fn(on);
    }

    void LEDManager::set_aux_led_status(int status) {
        bool on = status != STATUS_ON;
        if (OHDPlatform::instance().is_rpi()) {
            set_led_status(on, rpi::toggle_secondary_led);
        } else if (OHDPlatform::instance().is_zero3w()) {
            set_led_status(on, zero3w::toggle_secondary_led);
        } else if (OHDPlatform::instance().is_radxa_cm3()) {
            set_led_status(on, radxacm3::toggle_secondary_led);
        }
    }

    void LEDManager::set_rgb_led_status(int status, int color) {
        bool on = status != STATUS_ON;
        if (OHDPlatform::instance().is_rpi()) {
            set_led_status(on, rpi::toggle_secondary_led);
        } else if (OHDPlatform::instance().is_zero3w()) {
            set_led_status(on, zero3w::toggle_secondary_led);
        } else if (OHDPlatform::instance().is_radxa_cm3()) {
            set_led_status(on, radxacm3::toggle_secondary_led);
        }
    }

    void LEDManager::set_secondary_led_status(int status) {
        bool on = status != STATUS_ON;
        if (OHDPlatform::instance().is_rpi()) {
            set_led_status(on, rpi::toggle_secondary_led);
        } else if (OHDPlatform::instance().is_zero3w()) {
            set_led_status(on, zero3w::toggle_secondary_led);
        } else if (OHDPlatform::instance().is_radxa_cm3()) {
            set_led_status(on, radxacm3::toggle_secondary_led);
        }
    }

    void LEDManager::set_primary_led_status(int status) {
        bool on = status != STATUS_ON;
        if (OHDPlatform::instance().is_rpi()) {
            set_led_status(on, rpi::toggle_primary_led);
        } else if (OHDPlatform::instance().is_zero3w()) {
            set_led_status(on, zero3w::toggle_primary_led);
        } else if (OHDPlatform::instance().is_radxa_cm3()) {
            set_led_status(on, radxacm3::toggle_primary_led);
        }
    }

    LEDManager::LEDManager() {
    }

    LEDManager::~LEDManager() {
        stop_blinking_primary_led(); // Ensure thread is stopped
        if (m_blinking_thread.joinable()) {
            m_blinking_thread.join();
        }
    }

    void LEDManager::loop() {
    }

    void LEDManager::set_status_okay() {
        std::string baseDir = "/sys/class/leds/";
        auto folders = listLedFoldersWithBrightness(baseDir);
        turnOnAllLeds(folders, baseDir);
    }

    void LEDManager::set_status_loading() {
        start_blinking_primary_led(1, 10);  // Blink at 1 Hz for 10 seconds
    }

    void LEDManager::set_status_error() {
        set_primary_led_status(STATUS_ON);
        set_secondary_led_status(STATUS_ON);
        m_has_error = true;
    }

    void LEDManager::start_blinking_primary_led(int frequency_hz, int duration_seconds) {
        std::lock_guard<std::mutex> lock(m_blinking_mutex);
        if (m_blinking_running) {
            // Already running
            return;
        }
        m_blinking_running = true;
        m_blinking_thread = std::thread(&LEDManager::blink_leds_thread, this, frequency_hz, duration_seconds);
    }

    void LEDManager::stop_blinking_primary_led() {
        {
            std::lock_guard<std::mutex> lock(m_blinking_mutex);
            m_blinking_running = false;
        }
        if (m_blinking_thread.joinable()) {
            m_blinking_thread.join();
        }
    }

    void LEDManager::blink_leds_thread(int frequency_hz, int duration_seconds) {
        auto delay_on = std::chrono::milliseconds(1000 / (frequency_hz * 2));
        auto end_time = std::chrono::steady_clock::now() + std::chrono::seconds(duration_seconds);

        while (m_blinking_running && std::chrono::steady_clock::now() < end_time) {
            set_primary_led_status(STATUS_ON);
            std::this_thread::sleep_for(delay_on);
            set_primary_led_status(STATUS_OFF);
            std::this_thread::sleep_for(delay_on);
        }

        set_primary_led_status(STATUS_OFF);  // Ensure LED is off after blinking
    }
}
