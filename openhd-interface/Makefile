
SRC_DIR = $(PWD)/src
LIB_DIR = $(PWD)/lib

INC_DIR = -I$(PWD)/inc -I$(PWD)/../openhd-common

ifeq ($(PREFIX),)
	PREFIX := /usr/local
endif

ifdef $(DESTDIR)
	$(DESTDIR) := $(DESTDIR)/
endif

SYSTEM_INCLUDE = $(PREFIX)/include
LOCAL_INCLUDE = $(PWD)/lib/include
LDFLAGS = -L$(PREFIX)/lib -lboost_filesystem -lboost_regex -lboost_system


openhd_interface: wifi.o main.o 
	g++ -g -pthread -o openhd_interface wifi.o main.o  $(LDFLAGS)

main.o: $(SRC_DIR)/main.cpp
	g++ -std=c++17 -Wno-psabi -g -c -pthread -I$(SYSTEM_INCLUDE) -I$(LOCAL_INCLUDE) $(INC_DIR) $(SRC_DIR)/main.cpp

wifi.o: $(SRC_DIR)/wifi.cpp
	g++ -std=c++17 -Wno-psabi -g -c -pthread -I$(SYSTEM_INCLUDE) -I$(LOCAL_INCLUDE) $(INC_DIR) $(SRC_DIR)/wifi.cpp

clean:
	rm -f *.o openhd_interface


.PHONY: install
install: openhd_interface
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -d $(DESTDIR)/etc/systemd/system
	install -m 755 openhd_interface $(DESTDIR)$(PREFIX)/bin/
	install -m 644 openhd_interface.service $(DESTDIR)/etc/systemd/system/

.PHONY: enable
enable: install
	systemctl enable openhd_interface
	systemctl start openhd_interface

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/openhd_interface
