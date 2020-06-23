_LDFLAGS := $(LDFLAGS) -lrt
CFLAGS := $(CFLAGS) -I../../wifibroadcast-base -I../../lib/mavlink_generated/include/mavlink/v2.0

all: RCParseCh

%.o: %.c *.h
	$(CC) $(CFLAGS) -O2 -std=gnu99 -c -o $@ $(_LDFLAGS)

clean :
	rm RCParseCh RCParseCh.o

install:
	install -m 755 -p RCParseCh ${DESTDIR}/usr/local/bin/
