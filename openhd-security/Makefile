
SRC_DIR = $(PWD)/src
INC_DIR = -I$(PWD)/inc

ifeq ($(PREFIX),)
	PREFIX := /usr/local
endif

ifdef $(DESTDIR)
	$(DESTDIR) := $(DESTDIR)/
endif

SYSTEM_INCLUDE = $(PREFIX)/include
LOCAL_INCLUDE = $(PWD)/lib/include
LDFLAGS = -L$(PREFIX)/lib -lboost_filesystem -lboost_regex -lboost_system -lsodium


openhd_security: main.o 
	g++ -g -pthread -o openhd_security main.o $(LDFLAGS)

main.o: $(SRC_DIR)/main.cpp
	g++ -std=c++17 -Wno-psabi -g -c -pthread -I$(SYSTEM_INCLUDE) -I$(LOCAL_INCLUDE) $(INC_DIR) $(SRC_DIR)/main.cpp


clean:
	rm -f *.o openhd_security


.PHONY: install
install: openhd_security
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -d $(DESTDIR)/etc/systemd/system
	install -m 755 openhd_security $(DESTDIR)$(PREFIX)/bin/
	install -m 644 openhd_security.service $(DESTDIR)/etc/systemd/system/

.PHONY: enable
enable: install
	systemctl enable openhd_security
	systemctl start openhd_security

.PHONY: uninstall
uninstall:
	rm -f $(PREFIX)/bin/openhd_security
