
_LDFLAGS := $(LDFLAGS) -lrt -lpcap -lsodium
_CFLAGS := $(CFLAGS) -Wall -O2 -I/home/pi/wifibroadcast-base 

all: rx tx keygen

%.o: %.c *.h
	$(CC) -std=gnu99 -c -o $@ $< $(_CFLAGS)

%.o: %.cpp *.hpp *.h
	$(CXX) -std=gnu++11 -c -o $@ $< $(_CFLAGS)

rx: rx.o radiotap.o fec.o wifibroadcast.o
	$(CXX) -o $@ $^ $(_LDFLAGS)

rx_rc: rx_rc.o radiotap.o fec.o wifibroadcast.o
	$(CXX) -o $@ $^ $(_LDFLAGS)


tx: tx.o fec.o wifibroadcast.o
	$(CXX) -o $@ $^ $(_LDFLAGS)

keygen: keygen.o
	$(CC) -o $@ $^ $(_LDFLAGS)

build_rpi: clean
	docker build rpi_docker -t wifibroadcast:rpi_raspbian
	docker run -i -t --rm -v $(PWD):/build -v $(PWD):/rpxc/sysroot/build wifibroadcast:rpi_raspbian make CFLAGS=--sysroot=/rpxc/sysroot LDFLAGS="--sysroot=/rpxc/sysroot" CXX=arm-linux-gnueabihf-g++ CC=arm-linux-gnueabihf-gcc
	mkdir -p dist
	tar czf dist/wifibroadcast_rpi.tar.gz tx rx keygen -C scripts tx_standalone.sh rx_standalone.sh

telem:
	python -m telemetry.server './rx -a 5601 -u 5600' './rx -a 14551 -u 14552'

clean:
	rm -rf rx tx keygen dist *~ *.o

