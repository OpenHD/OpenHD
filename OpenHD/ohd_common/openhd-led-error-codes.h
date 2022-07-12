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

    class LEDBlinker{
    public:
        // blink red led in 1 second intervalls, runs for duration seconds. Defaults to infinity (note the calling thread will be blocked then)
        static void blink_red_led(const std::string message,const std::chrono::seconds duration=DURATION_INFINITY){
            const auto start=std::chrono::steady_clock::now();
            while ((std::chrono::steady_clock::now()-start)<=duration){
                std::cout<<message<<"\n";
                toggle_red_led(false);
                std::this_thread::sleep_for(std::chrono::seconds(1));
                toggle_red_led(true);
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
        // For running in its own thread
        // Make sure to store a reference to this class, otherwise destruct will fail since thread is still running.
        explicit LEDBlinker(const std::string message,const std::chrono::seconds duration=DURATION_INFINITY):
        _duration(duration),_message(message){
            _blink_thread=std::make_unique<std::thread>(&LEDBlinker::run, this);
        }
    private:
        void run(){
            blink_red_led(_message,_duration);
        }
        std::unique_ptr<std::thread> _blink_thread= nullptr;
        const std::chrono::seconds _duration;
        const std::string _message;
        static constexpr auto DURATION_INFINITY=std::chrono::seconds(100*100*100*100);
    };
}

#endif //OPENHD_OPENHD_LED_ERROR_CODES_H
