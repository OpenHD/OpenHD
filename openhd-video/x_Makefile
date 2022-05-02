
SRC_DIR = $(PWD)/src
LIB_DIR = $(PWD)/lib

CFLAGS = -I$(PWD)/../lib/fmt/include  -I$(PREFIX)/include -I$(PWD)/inc -I$(PWD)/../openhd-common -I$(PWD)/../lib/c_library_v2_openhd `pkg-config --cflags gstreamer-base-1.0`

ifeq ($(PREFIX),)
	PREFIX := /usr/local
endif

ifdef $(DESTDIR)
	$(DESTDIR) := $(DESTDIR)/
endif

LDFLAGS = -L$(PWD)/../lib/fmt/build -lfmt -L$(PREFIX)/lib -lboost_filesystem -lboost_regex -lboost_system -lsystemd `pkg-config --libs gstreamer-base-1.0`

openhd_video: cameramicroservice.o control.o hotspot.o camerastream.o gstreamerstream.o main.o
	g++ -g -pthread -o openhd_video control.o hotspot.o cameramicroservice.o camerastream.o gstreamerstream.o main.o  $(LDFLAGS)

main.o: $(SRC_DIR)/main.cpp
	g++ -std=c++17 -Wno-psabi -g -c -pthread $(CFLAGS) $(SRC_DIR)/main.cpp

hotspot.o: $(SRC_DIR)/hotspot.cpp
	g++ -std=c++17 -Wno-psabi -g -c -pthread $(CFLAGS) $(SRC_DIR)/hotspot.cpp

control.o: $(SRC_DIR)/control.cpp
	g++ -std=c++17 -Wno-psabi -g -c -pthread $(CFLAGS) $(SRC_DIR)/control.cpp

mavlinkcontrol.o: $(SRC_DIR)/mavlinkcontrol.cpp
	g++ -std=c++17 -Wno-psabi -g -c -pthread $(CFLAGS) $(SRC_DIR)/mavlinkcontrol.cpp

cameramicroservice.o: $(SRC_DIR)/cameramicroservice.cpp
	g++ -std=c++17 -Wno-psabi -g -c -pthread $(CFLAGS) $(SRC_DIR)/cameramicroservice.cpp

camerastream.o: $(SRC_DIR)/camerastream.cpp
	g++ -std=c++17 -Wno-psabi -g -c -pthread $(CFLAGS) $(SRC_DIR)/camerastream.cpp

gstreamerstream.o: $(SRC_DIR)/gstreamerstream.cpp
	g++ -std=c++17 -Wno-psabi -g -c -pthread $(CFLAGS) $(SRC_DIR)/gstreamerstream.cpp

flironestream.o: $(SRC_DIR)/flironestream.cpp
	g++ -std=c++17 -Wno-psabi -g -c -pthread $(CFLAGS) $(SRC_DIR)/flironestream.cpp

seekstream.o: $(SRC_DIR)/seekstream.cpp
	g++ -std=c++17 -Wno-psabi -g -c -pthread $(CFLAGS) $(SRC_DIR)/seekstream.cpp


clean:
	rm -f *.o openhd_video


.PHONY: install
install: openhd_video
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -d $(DESTDIR)/etc/systemd/system
	install -m 755 openhd_video $(DESTDIR)$(PREFIX)/bin/
	install -m 644 openhd_video.service $(DESTDIR)/etc/systemd/system/

.PHONY: enable
enable: install
	systemctl enable openhd_video
	systemctl start openhd_video

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/openhd_video
