
# bring in the helper from the parent / src / HelperSources File
# kinda dirty (fixme but how)
HELPER_DIR := ../../src/HelperSources/

all: test udp_forwarder

test : test.cpp
	g++ -std=c++17 test.cpp UDPReceiver.cpp UDPSender.cpp -o test -lpthread -I $(HELPER_DIR)

udp_forwarder : udp_forwarder.cpp
	g++ -std=c++17 udp_forwarder.cpp UDPReceiver.cpp UDPSender.cpp -o udp_forwarder -lpthread -I $(HELPER_DIR)

clean:
	rm -rf test
	rm -rf udp_forwarder