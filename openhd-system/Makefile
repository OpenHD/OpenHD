
SRC_DIR = $(PWD)/src
LIB_DIR = $(PWD)/lib

INC_DIR = -I$(PWD)/inc -I$(PWD)/../openhd-common -I/usr/include/libnl3

ifeq ($(PREFIX),)
	PREFIX := /usr/local
endif

ifdef $(DESTDIR)
	$(DESTDIR) := $(DESTDIR)/
endif

SYSTEM_INCLUDE = $(PREFIX)/include
LOCAL_INCLUDE = $(PWD)/lib/include
LDFLAGS = -L$(PREFIX)/lib -lboost_filesystem -lboost_regex -lboost_system -lv4l2 `pkg-config --cflags --libs libnl-3.0 libnl-genl-3.0`


openhd_system: nl.o bcm2835.o profile.o wifi.o cameras.o platform.o main.o 
	g++ -g -pthread -o openhd_system nl.o bcm2835.o profile.o wifi.o cameras.o platform.o main.o $(LDFLAGS)

main.o: $(SRC_DIR)/main.cpp
	g++ -std=c++17 -Wno-psabi -g -c -pthread -I$(SYSTEM_INCLUDE) -I$(LOCAL_INCLUDE) $(INC_DIR) $(SRC_DIR)/main.cpp

platform.o: $(SRC_DIR)/platform.cpp
	g++ -std=c++17 -Wno-psabi -g -c -pthread -I$(SYSTEM_INCLUDE) -I$(LOCAL_INCLUDE) $(INC_DIR) $(SRC_DIR)/platform.cpp

cameras.o: $(SRC_DIR)/cameras.cpp
	g++ -std=c++17 -Wno-psabi -g -c -pthread -I$(SYSTEM_INCLUDE) -I$(LOCAL_INCLUDE) $(INC_DIR) $(SRC_DIR)/cameras.cpp

wifi.o: $(SRC_DIR)/wifi.cpp
	g++ -std=c++17 -Wno-psabi -g -c -pthread -I$(SYSTEM_INCLUDE) -I$(LOCAL_INCLUDE) $(INC_DIR) $(SRC_DIR)/wifi.cpp

profile.o: $(SRC_DIR)/profile.cpp
	g++ -std=c++17 -Wno-psabi -g -c -pthread -I$(SYSTEM_INCLUDE) -I$(LOCAL_INCLUDE) $(INC_DIR) $(SRC_DIR)/profile.cpp

bcm2835.o: $(LIB_DIR)/bcm2835.c
	gcc -g -c -pthread -I$(SYSTEM_INCLUDE) -I$(LOCAL_INCLUDE) $(INC_DIR) $(LIB_DIR)/bcm2835.c

nl.o: $(SRC_DIR)/nl.c
	gcc -g -c -pthread -I$(SYSTEM_INCLUDE) -I$(LOCAL_INCLUDE) $(INC_DIR) $(SRC_DIR)/nl.c


clean:
	rm -f *.o openhd_system


.PHONY: install
install: openhd_system
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -d $(DESTDIR)/etc/systemd/system
	install -m 755 openhd_system $(DESTDIR)$(PREFIX)/bin/
	install -m 644 openhd_system.service $(DESTDIR)/etc/systemd/system/

.PHONY: enable
enable: install
	systemctl enable openhd_system
	systemctl start openhd_system

.PHONY: uninstall
uninstall:
	rm -f $(PREFIX)/bin/openhd_microservice
