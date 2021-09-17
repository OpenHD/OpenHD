LDFLAGS=-lrt -lpcap -lwiringPi
CPPFLAGS=-Wall -O2 -march=armv6zk -mcpu=arm1176jzf-s -mtune=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -D _GNU_SOURCE
CFLAGS := $(CFLAGS) -I../lib/mavlink_generated/include/mavlink/v2.0/

all: rx_rc_telemetry_buf rx rx_rc_telemetry rssirx rssitx tx_rawsock tx_telemetry rx_status tracker rssilogger syslogger channelscan check_alive rssi_forward rssi_qgc_forward wifiscan wifibackgroundscan sharedmem_init_rx sharedmem_init_tx

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $< $(CPPFLAGS)
	

rx: rx.o openhdlib.o radiotap.o fec.o
	gcc -o $@ $^ $(LDFLAGS)

rx_rc_telemetry: rx_rc_telemetry.o openhdlib.o radiotap_rc.o
	gcc -o $@ $^ $(LDFLAGS)

rx_rc_telemetry_buf: rx_rc_telemetry_buf.o openhdlib.o radiotap.o
	gcc -o $@ $^ $(LDFLAGS)

rssirx: rssirx.o openhdlib.o radiotap_rc.o
	gcc -o $@ $^ $(LDFLAGS)

rssitx: rssitx.o openhdlib.o
	gcc -o $@ $^ $(LDFLAGS)



tx_rawsock: tx_rawsock.o openhdlib.o fec.o
	gcc -o $@ $^ $(LDFLAGS)

tx_telemetry: tx_telemetry.o openhdlib.o fec.o
	gcc -o $@ $^ $(LDFLAGS)

rx_status: rx_status.o
	gcc -o $@ $^ $(LDFLAGS)

tracker: tracker.o
	gcc -o $@ $^ $(LDFLAGS)

rssilogger: rssilogger.o
	gcc -o $@ $^ $(LDFLAGS)

syslogger: syslogger.o
	gcc -o $@ $^ $(LDFLAGS)

channelscan: channelscan.o
	gcc -o $@ $^ $(LDFLAGS)

check_alive: check_alive.o
	gcc -o $@ $^ $(LDFLAGS)

rssi_forward: rssi_forward.o
	gcc -o $@ $^ $(LDFLAGS)

rssi_qgc_forward: rssi_qgc_forward.o
	gcc -o $@ $^ $(LDFLAGS)

wifiscan: wifiscan.o radiotap.o
	gcc -o $@ $^ $(LDFLAGS)

wifibackgroundscan: wifibackgroundscan.o radiotap.o
	gcc -o $@ $^ $(LDFLAGS)

sharedmem_init_rx: sharedmem_init_rx.o openhdlib.o
	gcc -o $@ $^ $(LDFLAGS)

sharedmem_init_tx: sharedmem_init_tx.o openhdlib.o
	gcc -o $@ $^ $(LDFLAGS)

clean:
	rm -f rx_rc_telemetry_buf rx rx_rc_telemetry rssirx rssitx tx_rawsock tx_telemetry rx_status tracker rssilogger syslogger channelscan check_alive rssi_forward rssi_qgc_forward wifiscan wifibackgroundscan sharedmem_init_rx sharedmem_init_tx *~ *.o

install:
	install -m 755 -p rx ${DESTDIR}/usr/local/bin/
	install -m 755 -p rx_rc_telemetry ${DESTDIR}/usr/local/bin/
	install -m 755 -p rx_rc_telemetry_buf ${DESTDIR}/usr/local/bin/
	install -m 755 -p rssirx ${DESTDIR}/usr/local/bin/
	install -m 755 -p rssitx ${DESTDIR}/usr/local/bin/
	install -m 755 -p tx_rawsock ${DESTDIR}/usr/local/bin/
	install -m 755 -p tx_telemetry ${DESTDIR}/usr/local/bin/
	install -m 755 -p rx_status ${DESTDIR}/usr/local/bin/
	install -m 755 -p tracker ${DESTDIR}/usr/local/bin/
	install -m 755 -p rssilogger ${DESTDIR}/usr/local/bin/
	install -m 755 -p syslogger ${DESTDIR}/usr/local/bin/
	install -m 755 -p channelscan ${DESTDIR}/usr/local/bin/
	install -m 755 -p check_alive ${DESTDIR}/usr/local/bin/
	install -m 755 -p rssi_forward ${DESTDIR}/usr/local/bin/
	install -m 755 -p rssi_qgc_forward ${DESTDIR}/usr/local/bin/
	install -m 755 -p wifiscan ${DESTDIR}/usr/local/bin/
	install -m 755 -p wifibackgroundscan ${DESTDIR}/usr/local/bin/
	install -m 755 -p sharedmem_init_rx ${DESTDIR}/usr/local/bin/
	install -m 755 -p sharedmem_init_tx ${DESTDIR}/usr/local/bin/
