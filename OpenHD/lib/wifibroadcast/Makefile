# CMAKE generated file: DO NOT EDIT!
VERSION ?= $(shell ./version.py)
COMMIT ?= $(shell git rev-parse HEAD)

export VERSION COMMIT

_LDFLAGS := $(LDFLAGS) -lrt -lpcap -lsodium -lpthread
# WFB_VERSION is date and time and the last commit of this branch
_CFLAGS := $(CFLAGS)  -O2 -DWFB_VERSION='"$(VERSION)-$(shell /bin/bash -c '_tmp=$(COMMIT); echo $${_tmp::8}')"'

# depending on the architecture we need the right flags for optimized fec encoding/decoding
uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')
ifeq ($(uname_S),Linux)
	uname_M := $(shell sh -c 'uname -m 2>/dev/null || echo not')
	ifeq ($(uname_M),x86_64)
		_CFLAGS += -mavx2 -faligned-new=256
	else ifeq ($(uname_M),armv7l)
 		_CFLAGS += -mfpu=neon -march=armv7-a -marm
	endif
endif

all_bin: wfb_rx wfb_tx wfb_keygen unit_test benchmark udp_generator_validator socket_helper_test
all: all_bin gs.key

# Just compile everything as c++ code
# compile radiotap
src/external/radiotap/radiotap.o: src/external/radiotap/radiotap.c src/external/radiotap/*.h
	$(CC) $(_CFLAGS) -std=c++17 -c -o $@ $<

# compile the (general) fec part
src/external/fec/fec.o: src/external/fec/fec.cpp src/external/fec/*.h src/external/fec/gf_optimized/*.h src/external/fec/gf_simple/*.h
	$(CXX) $(_CFLAGS) -std=c++17 -c -o $@ $<

# the c++ part
src/%.o: src/%.cpp
	$(CXX) $(_CFLAGS) -std=c++17 -c -o $@ $<
executables/%.o: executables/%.cpp
	$(CXX) $(_CFLAGS) -std=c++17 -c -o $@ $<

wfb_rx: executables/rx.o src/WBReceiver.o src/external/radiotap/radiotap.o src/external/fec/fec.o
	$(CXX) -o $@ $^ $(_LDFLAGS)

wfb_tx: executables/tx.o src/WBTransmitter.o src/external/radiotap/radiotap.o src/external/fec/fec.o
	$(CXX) -o $@ $^ $(_LDFLAGS)

unit_test: executables/unit_test.o src/external/fec/fec.o
	$(CXX) -o $@ $^ $(_LDFLAGS)

benchmark: executables/benchmark.o src/external/fec/fec.o
	$(CXX) -o $@ $^ $(_LDFLAGS)

udp_generator_validator: executables/udp_generator_validator.o
	$(CXX) -o $@ $^ $(_LDFLAGS)

socket_helper_test: executables/socket_helper_test.o
	$(CXX) -o $@ $^ $(_LDFLAGS)

wfb_keygen: executables/keygen.o
	$(CC) -o $@ $^ $(_LDFLAGS)

gs.key: wfb_keygen
	@if ! [ -f gs.key ]; then ./wfb_keygen; fi

clean:
	rm -rf env wfb_rx wfb_tx wfb_keygen unit_test benchmark udp_generator_validator src/*.o src/external/fec/*.o src/external/radiotap/*.o src/executables/*.o


# experimental
.PHONY: install
install:
	cp -f wfb_tx wfb_rx benchmark udp_generator_validator unit_test wfb_keygen $(TARGET_DIR)/usr/local/bin/

.PHONY: uninstall
uninstall:
	rm -f $(TARGET_DIR)/usr/local/bin/wfb_*