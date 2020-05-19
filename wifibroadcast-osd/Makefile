CPPFLAGS+= -I/home/pi/wifibroadcast-base -I/opt/vc/include/ -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux
LDFLAGS+= -lfreetype -lz -lboost_system -lboost_regex -lboost_filesystem -lboost_thread -lpthread
LDFLAGS+=-L/opt/vc/lib/ -lbrcmGLESv2 -lbrcmEGL -lopenmaxil -lbcm_host -lvcos -lvchiq_arm -lpthread -lrt -lm -lshapes
CFLAGS := $(CFLAGS) -I/usr/local/include/mavlink

all: osd

/tmp/%.o: %.cpp
	g++ $(CFLAGS) -c -o $@ $< $(CPPFLAGS)

osd: /tmp/main.o /tmp/render.o /tmp/telemetry.o /tmp/flightmode.o /tmp/frsky.o /tmp/ltm.o /tmp/mavlink.o /tmp/smartport.o /tmp/vot.o /tmp/settings.o
	g++ -o /usr/local/bin/$@ $^ $(LDFLAGS)

clean:
	rm -f /usr/local/bin/osd /tmp/*.o /tmp/*~
