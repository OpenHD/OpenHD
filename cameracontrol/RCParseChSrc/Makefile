_LDFLAGS := $(LDFLAGS) -lrt
CFLAGS := $(CFLAGS) -I/home/pi/wifibroadcast-base -I/usr/local/include/mavlink

all: RCParseCh

%.o: %.c *.h
	$(CC) $(CFLAGS) -O2 -std=gnu99 -c -o $@ $(_LDFLAGS)

clean :
	rm RCParseCh RCParseCh.o
