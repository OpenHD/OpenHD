
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
LDFLAGS = -L$(PREFIX)/lib -lboost_filesystem -lboost_regex -lboost_system -lv4l2


openhd_system: wifi.o cameras.o platform.o main.o 
	g++ -g -pthread -o openhd_system wifi.o cameras.o platform.o main.o $(LDFLAGS)

main.o: $(SRC_DIR)/main.cpp
	g++ -std=c++17 -Wno-psabi -g -c -pthread -I$(SYSTEM_INCLUDE) -I$(LOCAL_INCLUDE) $(INC_DIR) $(SRC_DIR)/main.cpp

platform.o: $(SRC_DIR)/platform.cpp
	g++ -std=c++17 -Wno-psabi -g -c -pthread -I$(SYSTEM_INCLUDE) -I$(LOCAL_INCLUDE) $(INC_DIR) $(SRC_DIR)/platform.cpp

cameras.o: $(SRC_DIR)/cameras.cpp
	g++ -std=c++17 -Wno-psabi -g -c -pthread -I$(SYSTEM_INCLUDE) -I$(LOCAL_INCLUDE) $(INC_DIR) $(SRC_DIR)/cameras.cpp

wifi.o: $(SRC_DIR)/wifi.cpp
	g++ -std=c++17 -Wno-psabi -g -c -pthread -I$(SYSTEM_INCLUDE) -I$(LOCAL_INCLUDE) $(INC_DIR) $(SRC_DIR)/wifi.cpp


clean:
	rm -f *.o openhd_system


.PHONY: install
install: openhd_system
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -d $(DESTDIR)/etc/systemd/system
	install -m 755 openhd_system $(DESTDIR)$(PREFIX)/bin/
	install -m 644 openhd_system@.service $(DESTDIR)/etc/systemd/system/

.PHONY: enable
enable: install
	systemctl enable openhd_system
	systemctl start openhd_system

.PHONY: uninstall
uninstall:
	rm -f $(PREFIX)/bin/openhd_microservice
