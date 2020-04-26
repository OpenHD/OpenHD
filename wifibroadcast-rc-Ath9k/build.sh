ionice -c 3 nice dos2unix -n /boot/joyconfig.txt /tmp/rctx.h > /dev/null 2>&1
g++ -I/home/pi/wifibroadcast-base rctx.cpp -o rctx `sdl-config --libs` `sdl-config --cflags` -lrt -lwiringPi -lpcap -lboost_system -lboost_regex -lboost_filesystem -lboost_thread -lpthread
g++ -I/home/pi/wifibroadcast-base rcswitches.cpp -o rcswitches `sdl-config --libs` `sdl-config --cflags` -lrt 