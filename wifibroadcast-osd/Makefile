CPPFLAGS+= -I../openvg -I../wifibroadcast-base -I/opt/vc/include/ -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux
LDFLAGS+= -lfreetype -lz -lboost_system -lboost_regex -lboost_filesystem -lboost_thread -lpthread
LDFLAGS+= -L../openvg -L/opt/vc/lib/ -lbrcmGLESv2 -lbrcmEGL -lopenmaxil -lbcm_host -lvcos -lvchiq_arm -lpthread -lrt -lm -lshapes
CFLAGS := $(CFLAGS) -I../lib/mavlink_generated/include/mavlink/v2.0/

all: osd

/tmp/%.o: %.cpp
	g++ $(CFLAGS) -c -o $@ $< $(CPPFLAGS)

osd: /tmp/main.o /tmp/render.o /tmp/telemetry.o /tmp/flightmode.o /tmp/frsky.o /tmp/ltm.o /tmp/mavlink.o /tmp/smartport.o /tmp/vot.o /tmp/settings.o
	g++ -o $@ $^ $(LDFLAGS)

clean:
	rm -f osd /tmp/*.o /tmp/*~

install:
	install -m 755 -p osd ${DESTDIR}/usr/local/bin/
