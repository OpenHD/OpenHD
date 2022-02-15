
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

openhd_settings_m: settingsmanager.o main.o
	g++ -g -pthread -o openhd_settings_m settingsmanager.o main.o  $(LDFLAGS)

main.o: $(SRC_DIR)/main.cpp
	g++ -std=c++17 -Wno-psabi -g -c -pthread $(CFLAGS) $(SRC_DIR)/main.cpp

settingsmanager.o: $(SRC_DIR)/settingsmanager.cpp
	g++ -std=c++17 -Wno-psabi -g -c -pthread $(CFLAGS) $(SRC_DIR)/settingsmanager.cpp



clean:
	rm -f *.o openhd_settings_m


.PHONY: install
install: openhd_settings_m
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -d $(DESTDIR)/etc/systemd/system
	install -m 755 openhd_settings_m $(DESTDIR)$(PREFIX)/bin/
	install -m 644 openhd_settings_m.service $(DESTDIR)/etc/systemd/system/

.PHONY: enable
enable: install
	systemctl enable openhd_settings_m
	systemctl start openhd_settings_m

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/openhd_settings_m
