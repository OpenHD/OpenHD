all: qstatus

%.o: %.c
	gcc -c -o $@ $< $(CPPFLAGS)

qstatus: qstatus.o
	gcc -o $@ $^

clean:
	rm -f qstatus *.o *~

install:
	install -m 755 -p qstatus ${DESTDIR}/usr/local/bin/

