ionice -c 3 nice dos2unix -n /boot/joyconfig.txt /tmp/rctx.h > /dev/null 2>&1
gcc -lrt -I/home/pi/wifibroadcast-base rctx.c -o rctx `sdl-config --libs` `sdl-config --cflags`
gcc -lrt -I/home/pi/wifibroadcast-base rcswitches.c -o rcswitches `sdl-config --libs` `sdl-config --cflags`