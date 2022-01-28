VERSION ?= $(shell ./version.py)
COMMIT ?= $(shell git rev-parse HEAD)

export VERSION COMMIT

_LDFLAGS := $(LDFLAGS) -lrt -lpcap -lsodium
# WFB_VERSION is date and time and the last commit of this branch
_CFLAGS := $(CFLAGS)  -O2 -DWFB_VERSION='"$(VERSION)-$(shell /bin/bash -c '_tmp=$(COMMIT); echo $${_tmp::8}')"'

all_bin: wfb_rx wfb_tx wfb_keygen unit_test benchmark udp_generator_validator
all: all_bin gs.key

# The non-c++ part
src/ExternalCSources/%.o: src/ExternalCSources/radiotap/%.c src/ExternalCSources/radiotap/*.h
	$(CC) $(_CFLAGS) -std=gnu99 -c -o $@ $<

#src/ExternalCSources/%.o: src/ExternalCSources/fec/%.c src/ExternalCSources/fec/*.h
#	$(CC) $(_CFLAGS) -Werror -std=gnu99 -c -o $@ $<


# the c++ part
src/%.o: src/%.cpp src/*.hpp
	$(CXX) $(_CFLAGS) -std=c++17 -c -o $@ $<

wfb_rx: src/rx.o src/ExternalCSources/radiotap/radiotap.o src/ExternalCSources/fec/fec.o
	$(CXX) -o $@ $^ $(_LDFLAGS)

wfb_tx: src/tx.o src/ExternalCSources/radiotap/radiotap.o src/ExternalCSources/fec/fec.o
	$(CXX) -o $@ $^ $(_LDFLAGS)

unit_test: src/unit_test.o src/ExternalCSources/fec/fec.o
	$(CXX) -o $@ $^ $(_LDFLAGS)

benchmark: src/benchmark.o src/ExternalCSources/fec/fec.o
	$(CXX) -o $@ $^ $(_LDFLAGS)

udp_generator_validator: src/udp_generator_validator.o
	$(CXX) -o $@ $^ $(_LDFLAGS)

wfb_keygen: src/keygen.o
	$(CC) -o $@ $^ $(_LDFLAGS)

gs.key: wfb_keygen
	@if ! [ -f gs.key ]; then ./wfb_keygen; fi

clean:
	rm -rf env wfb_rx wfb_tx wfb_keygen unit_test benchmark udp_generator_validator src/*.o src/ExternalCSources/fec/*.o src/ExternalCSources/radiotap/*.o


# experimental
.PHONY: install
install:
	cp -f wfb_tx $(TARGET_DIR)/usr/bin/

.PHONY: uninstall
uninstall:
	rm -f $(TARGET_DIR)/usr/bin/wfb_tx