CPPFLAGS = -I$(PWD)/../openhd-common

ifeq ($(PREFIX),)
	PREFIX := /usr/local
endif

ifdef $(DESTDIR)
	$(DESTDIR) := $(DESTDIR)/
endif


all: qstatus

%.o: %.c
	g++ -c -o $@ $< $(CPPFLAGS)

qstatus: qstatus.o
	g++ -o $@ $^

clean:
	rm -f qstatus *.o *~

install:
	install -m 755 -p qstatus $(DESTDIR)$(PREFIX)/bin/
