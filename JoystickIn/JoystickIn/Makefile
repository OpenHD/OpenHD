_LDFLAGS := $(LDFLAGS) -lrt
 all: processUDP

%.o: %.c *.h
	$(CC) -std=gnu99 -c -o $@ $^ $(_LDFLAGS)

processUDP: processUDP.o 
	$(CXX) -o $@ $^ $(_LDFLAGS)

clean :
	rm processUDP processUDP.o
