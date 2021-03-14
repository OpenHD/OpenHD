INC_DIR = -I$(PREFIX)/include -I$(PWD)/inc -I$(PWD)/../openhd-common -I$(PWD)/../lib/mavlink_generated/include/mavlink/v2.0
SRC_DIR = ./src

ifeq ($(PREFIX),)
	PREFIX := /usr/local
endif

ifdef $(DESTDIR)
	$(DESTDIR) := $(DESTDIR)/
endif


LDFLAGS = -L$(PREFIX)/lib -lboost_filesystem -lboost_system -lboost_program_options -lboost_regex


openhd_telemetry: serialendpoint.o router.o endpoint.o udpendpoint.o tcpendpoint.o main.o 
	g++ -std=c++17 -g -pthread -o openhd_telemetry serialendpoint.o router.o endpoint.o udpendpoint.o tcpendpoint.o main.o $(LDFLAGS)

main.o: $(SRC_DIR)/main.cpp
	g++ -std=c++17  -g -c -pthread $(INC_DIR) $(SRC_DIR)/main.cpp

router.o: $(SRC_DIR)/router.cpp
	g++ -std=c++17 -g -c -pthread $(INC_DIR) $(SRC_DIR)/router.cpp

endpoint.o: $(SRC_DIR)/endpoint.cpp
	g++ -std=c++17 -g -c -pthread $(INC_DIR) $(SRC_DIR)/endpoint.cpp

tcpendpoint.o: $(SRC_DIR)/tcpendpoint.cpp
	g++ -std=c++17 -g -c -pthread $(INC_DIR) $(SRC_DIR)/tcpendpoint.cpp

udpendpoint.o: $(SRC_DIR)/udpendpoint.cpp
	g++ -std=c++17 -g -c -pthread $(INC_DIR) $(SRC_DIR)/udpendpoint.cpp


serialendpoint.o: $(SRC_DIR)/serialendpoint.cpp
	g++ -std=c++17 -g -c -pthread $(INC_DIR) $(SRC_DIR)/serialendpoint.cpp


clean:
	rm -f *.o openhd_telemetry

.PHONY: install
install: openhd_telemetry
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -d $(DESTDIR)/etc/systemd/system
	install -m 755 openhd_telemetry $(DESTDIR)$(PREFIX)/bin/
	install -m 644 openhd_telemetry@.service $(DESTDIR)/etc/systemd/system/
	install -d $(DESTDIR)/etc/openhd

.PHONY: enable
enable: install
	systemctl enable openhd_telemetry
	systemctl start openhd_telemetry

.PHONY: uninstall
uninstall:
	rm -f $(PREFIX)/bin/openhd_telemetry
