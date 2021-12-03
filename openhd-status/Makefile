
SRC_DIR = $(PWD)/src
LIB_DIR = $(PWD)/lib

CFLAGS = -I$(PWD)/../openhd-common -I$(PWD)/../lib/fmt/include  -I$(PREFIX)/include -I$(PWD)/inc -I$(PWD)/../openhd-common -I$(PWD)/../lib/mavlink_generated/include/mavlink/v2.0 `pkg-config --cflags gstreamer-base-1.0`

ifeq ($(PREFIX),)
	PREFIX := /usr/local
endif

ifdef $(DESTDIR)
	$(DESTDIR) := $(DESTDIR)/
endif

LDFLAGS = -L$(PWD)/../lib/fmt/build -lfmt -L$(PREFIX)/lib -lboost_filesystem -lboost_regex -lboost_system -lsystemd `pkg-config --libs gstreamer-base-1.0`

all: qstatus openhd_status

qstatus: qstatus.c
	g++ -o $@ $^ $(CFLAGS)
	
openhd_status: statusmicroservice.o main.o
	g++ -g -pthread -o openhd_status statusmicroservice.o main.o  $(LDFLAGS)

main.o: $(SRC_DIR)/main.cpp
	g++ -std=c++17 -Wno-psabi -g -c -pthread $(CFLAGS) $(SRC_DIR)/main.cpp


statusmicroservice.o: $(SRC_DIR)/statusmicroservice.cpp
	g++ -std=c++17 -Wno-psabi -g -c -pthread $(CFLAGS) $(SRC_DIR)/statusmicroservice.cpp



clean:
	rm -f *.o openhd_status qstatus


.PHONY: install
install: openhd_status
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -d $(DESTDIR)/etc/systemd/system
	install -m 755 openhd_status $(DESTDIR)$(PREFIX)/bin/
	install -m 644 openhd_status.service $(DESTDIR)/etc/systemd/system/

.PHONY: enable
enable: install
	systemctl enable openhd_status
	systemctl start openhd_status

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/openhd_status
