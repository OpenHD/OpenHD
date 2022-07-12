//
// Created by consti10 on 12.07.22.
//

#ifndef OPENHD_OPENHD_LED_ERROR_CODES_H
#define OPENHD_OPENHD_LED_ERROR_CODES_H

#include "openhd-util.hpp"
#include <chrono>
#include <thread>

namespace openhd::rpi{
    // so far, I have only tested this on the RPI 4 and CM4
    static void toggle_red_led(const bool on){
        int ret;
        if(on){
            OHDUtil::run_command("echo 1 > /sys/class/leds/led1/brightness",{});
        }else{
            OHDUtil::run_command("echo 0 > /sys/class/leds/led1/brightness",{});
        }
    }
    // I think the green led only supports on/off on the 4th generation pis
    static void toggle_green_led(const bool on){
        int ret;
        if(on){
            OHDUtil::run_command("echo 1 > /sys/class/leds/led0/brightness",{});
        }else{
            OHDUtil::run_command("echo 0 > /sys/class/leds/led0/brightness",{});
        }
    }

    // R.N only to show no connected wifi card, which requires a reboot to fix.
    class LEDBlinker{
    public:
        // toggle red led off, wait for delay, then toggle it on,wait for delay
        static void red_led_on_off_delayed(const std::chrono::milliseconds& delay){
            toggle_red_led(false);
            std::this_thread::sleep_for(delay);
            toggle_red_led(true);
            std::this_thread::sleep_for(delay);
        }
        // One on / off sequence is often not enough signal for the user, repeat the sequence for a given amount of time
        // blink red led in X second intervalls, runs for duration seconds. Defaults to infinity (note the calling thread will be blocked then)
        static void blink_red_led(const std::string& message,const std::chrono::seconds duration=DURATION_INFINITY){
            const auto start=std::chrono::steady_clock::now();
            while ((std::chrono::steady_clock::now()-start)<=duration){
                std::cout<<message<<"\n";
                red_led_on_off_delayed(std::chrono::seconds(1));
            }
        }
        // For running in its own thread
        // Make sure to store a reference to this class, otherwise destruct will fail since thread is still running.
        explicit LEDBlinker(const std::string& message,const std::chrono::seconds duration=DURATION_INFINITY):
                _message(message),_duration(duration){
            _blink_thread=std::make_unique<std::thread>(&LEDBlinker::run, this);
        }
        static constexpr auto DURATION_INFINITY=std::chrono::seconds(100*100*100*100);
    private:
        void run(){
            blink_red_led(_message,_duration);
        }
        const std::string _message;
        const std::chrono::seconds _duration;
        std::unique_ptr<std::thread> _blink_thread= nullptr;
    };
}

#endif //OPENHD_OPENHD_LED_ERROR_CODES_H
