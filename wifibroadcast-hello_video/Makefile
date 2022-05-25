CFLAGS+=-DSTANDALONE -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -DTARGET_POSIX -D_LINUX -fPIC -DPIC -D_REENTRANT -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -U_FORTIFY_SOURCE -Wall -g -DHAVE_LIBOPENMAX=2 -DOMX -DOMX_SKIP64BIT -ftree-vectorize -pipe -DUSE_EXTERNAL_OMX -DHAVE_LIBBCM_HOST -DUSE_EXTERNAL_LIBBCM_HOST -DUSE_VCHIQ_ARM -Wno-psabi

LDFLAGS+=-L/opt/vc/lib/ -lbrcmGLESv2 -lbrcmEGL -lopenmaxil -lbcm_host -lvcos -lvchiq_arm -lpthread -lrt -lm -L/opt/vc/src/hello_pi/libs/ilclient -L/opt/vc/src/hello_pi/libs/vgfont -L/opt/vc/src/hello_pi/libs/revision

INCLUDES+=-I/opt/vc/include/ -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux -I./ -I/opt/vc/src/hello_pi/libs/ilclient -I/opt/vc/src/hello_pi/libs/vgfont -I/opt/vc/src/hello_pi/libs/revision

LDFLAGS+=-lilclient


%.o: %.c
	gcc $(CFLAGS) -c -o $@ $< $(CPPFLAGS) $(INCLUDES)


all: hello_video.bin.30-mm hello_video.bin.48-mm hello_video.bin.240-befi hello_video.bin.player

hello_video.bin.30-mm: video-30mm.o
	gcc -o $@ $^ $(LDFLAGS)

hello_video.bin.48-mm: video-48mm.o
	gcc -o $@ $^ $(LDFLAGS)

hello_video.bin.240-befi: video-240befi.o
	gcc -o $@ $^ $(LDFLAGS)

hello_video.bin.player: video.player.o
	gcc -o $@ $^ $(LDFLAGS)

install:
	install -m 755 -p hello_video.bin.30-mm ${DESTDIR}/usr/local/bin/
	install -m 755 -p hello_video.bin.48-mm ${DESTDIR}/usr/local/bin/
	install -m 755 -p hello_video.bin.240-befi ${DESTDIR}/usr/local/bin/
	install -m 755 -p hello_video.bin.player ${DESTDIR}/usr/local/bin/

clean:
	rm hello_video.bin.30-mm hello_video.bin.48-mm hello_video.bin.240-befi hello_video.bin.player

