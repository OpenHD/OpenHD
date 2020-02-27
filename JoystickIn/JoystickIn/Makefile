_LDFLAGS := $(LDFLAGS) -lrt
CFLAGS := $(CFLAGS) -I/home/pi/wifibroadcast-base -I/usr/local/include/mavlink
 
all: processUDP

%.o: %.c *.h
	$(CC) $(CFLAGS) -std=gnu99 -c -o $@ $^ $(_LDFLAGS)

processUDP: processUDP.o 
	$(CXX) -o $@ $^ $(_LDFLAGS)

clean :
	rm processUDP processUDP.o
