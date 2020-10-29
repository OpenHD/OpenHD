g++ -std=c++11 -I../wifibroadcast-base rctx.cpp -o rctx `sdl-config --libs` `sdl-config --cflags` -lrt -lpcap -lboost_system -lboost_regex -lboost_filesystem -lboost_thread -lpthread
g++ -std=c++11 -I../wifibroadcast-base rcswitches.cpp -o rcswitches `sdl-config --libs` `sdl-config --cflags` -lrt 
gcc -lrt -I../wifibroadcast-base JoystickSender.c -o JoystickSender `sdl-config --libs` `sdl-config --cflags`
