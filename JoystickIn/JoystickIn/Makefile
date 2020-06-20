_LDFLAGS := $(LDFLAGS) -lrt
CFLAGS := $(CFLAGS) -I../../wifibroadcast-base -I../../lib/mavlink_generated/include/mavlink/v2.0/
 
all: processUDP

%.o: %.c *.h
	$(CC) $(CFLAGS) -std=gnu99 -c -o $@ $^ $(_LDFLAGS)

processUDP: processUDP.o 
	$(CXX) -o $@ $^ $(_LDFLAGS)

clean :
	rm processUDP processUDP.o

install:
	install -m 755 -p processUDP ${DESTDIR}/usr/local/bin/
