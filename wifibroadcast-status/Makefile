CPPFLAGS+= -I../openvg -I/opt/vc/include/ -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux
LDFLAGS+= -lfreetype -lz
LDFLAGS+= -L../openvg -L/opt/vc/lib/ -lbrcmGLESv2 -lbrcmEGL -lopenmaxil -lbcm_host -lvcos -lvchiq_arm -lpthread -lrt -lm -lshapes

all: wbc_status qstatus

%.o: %.c
	gcc -c -o $@ $< $(CPPFLAGS)
 

wbc_status: wbc_status.o
	gcc -o $@ $^ $(LDFLAGS)

qstatus: qstatus.o
	gcc -o $@ $^

clean:
	rm -f wbc_status qstatus *.o *~

install:
	install -m 755 -p wbc_status ${DESTDIR}/usr/local/bin/
	install -m 755 -p qstatus ${DESTDIR}/usr/local/bin/

