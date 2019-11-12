LDFLAGS=-lrt -lpcap -lwiringPi
CPPFLAGS=-Wall -O2 -march=armv6zk -mcpu=arm1176jzf-s -mtune=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -D _GNU_SOURCE

all: rx_rc_telemetry_buf rx rx_rc_telemetry rssirx rssitx tx_rawsock tx_telemetry tx_measure rx_status tracker rssilogger syslogger channelscan check_alive rssi_forward rssi_qgc_forward wifiscan wifibackgroundscan sharedmem_init_rx sharedmem_init_tx

%.o: %.c
	gcc -c -o $@ $< $(CPPFLAGS)


rx: rx.o lib.o radiotap.o fec.o
	gcc -o $@ $^ $(LDFLAGS)

rx_rc_telemetry: rx_rc_telemetry.o lib.o radiotap.o
	gcc -o $@ $^ $(LDFLAGS)

rx_rc_telemetry_buf: rx_rc_telemetry_buf.o lib.o radiotap.o
	gcc -o $@ $^ $(LDFLAGS)

rssirx: rssirx.o lib.o radiotap.o
	gcc -o $@ $^ $(LDFLAGS)

rssitx: rssitx.o lib.o radiotap.o
	gcc -o $@ $^ $(LDFLAGS)



tx_rawsock: tx_rawsock.o lib.o fec.o
	gcc -o $@ $^ $(LDFLAGS)

tx_telemetry: tx_telemetry.o lib.o fec.o
	gcc -o $@ $^ $(LDFLAGS)

tx_measure: tx_measure.o lib.o fec.o
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

sharedmem_init_rx: sharedmem_init_rx.o lib.o
	gcc -o $@ $^ $(LDFLAGS)

sharedmem_init_tx: sharedmem_init_tx.o lib.o
	gcc -o $@ $^ $(LDFLAGS)

clean:
	rm -f rx_rc_telemetry_buf rx rx_rc_telemetry rssirx rssitx tx_rawsock tx_telemetry tx_measure rx_status tracker rssilogger syslogger channelscan check_alive rssi_forward rssi_qgc_forward wifiscan wifibackgroundscan sharedmem_init_rx sharedmem_init_tx *~ *.o
