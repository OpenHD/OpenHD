
SRC_DIR = $(PWD)/src
LIB_DIR = $(PWD)/lib

CFLAGS = -I$(PWD)/../lib/fmt/include  -I$(PREFIX)/include -I$(PWD)/inc -I$(PWD)/../openhd-common -I$(PWD)/../lib/mavlink_generated/include/mavlink/v2.0 `pkg-config --cflags gstreamer-base-1.0`

ifeq ($(PREFIX),)
	PREFIX := /usr/local
endif

ifdef $(DESTDIR)
	$(DESTDIR) := $(DESTDIR)/
endif

LDFLAGS = -L$(PWD)/../lib/fmt/build -lfmt -L$(PREFIX)/lib -lboost_filesystem -lboost_regex -lboost_system -lsystemd `pkg-config --libs gstreamer-base-1.0`

openhd_power: powermicroservice.o main.o
	g++ -g -pthread -o openhd_power powermicroservice.o main.o  $(LDFLAGS)

main.o: $(SRC_DIR)/main.cpp
	g++ -std=c++17 -Wno-psabi -g -c -pthread $(CFLAGS) $(SRC_DIR)/main.cpp


powermicroservice.o: $(SRC_DIR)/powermicroservice.cpp
	g++ -std=c++17 -Wno-psabi -g -c -pthread $(CFLAGS) $(SRC_DIR)/powermicroservice.cpp



clean:
	rm -f *.o openhd_power


.PHONY: install
install: openhd_power
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -d $(DESTDIR)/etc/systemd/system
	install -m 755 openhd_power $(DESTDIR)$(PREFIX)/bin/
	install -m 644 openhd_power.service $(DESTDIR)/etc/systemd/system/

.PHONY: enable
enable: install
	systemctl enable openhd_power
	systemctl start openhd_power

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/openhd_power
