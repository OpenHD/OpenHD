CPPFLAGS+= -I/opt/vc/include/ -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux
LDFLAGS+= -lfreetype -lz
LDFLAGS+=-L/opt/vc/lib/ -lbrcmGLESv2 -lbrcmEGL -lopenmaxil -lbcm_host -lvcos -lvchiq_arm -lpthread -lrt -lm -lshapes

all: wbc_status qstatus

%.o: %.c
	gcc -c -o $@ $< $(CPPFLAGS)
 

wbc_status: wbc_status.o
	gcc -o /usr/local/bin/$@ $^ $(LDFLAGS)

qstatus: qstatus.o
	gcc -o /usr/local/bin/$@ $^

clean:
	rm -f wbc_status qstatus *.o *~

