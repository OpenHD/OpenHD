LDFLAGS=-lrt -lpcap
CPPFLAGS=-Wall -O2 -D _GNU_SOURCE
CFLAGS := $(CFLAGS) -I../lib/mavlink_generated/include/mavlink/v2.0/

all: tracker

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $< $(CPPFLAGS)

tracker: tracker.o
	gcc -o $@ $^ $(LDFLAGS)

clean:
	rm -f tracker *~ *.o

install:
	install -m 755 -p tracker ${DESTDIR}/usr/local/bin/
