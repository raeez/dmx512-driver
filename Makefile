CC = g++
LIBS = -lfftw3 -ljack -lm
CFLAGS = -O3 -I /Library/Frameworks/Jackmp.framework/Versions/Current/Headers/
TARGETS = fft.o dmx-eth.o

all: test

candyland: $(TARGETS)
	$(CC) $(LIBS) $(TARGETS) -o $@

.o: $*.cc
	$(CC) $(LIBS) $(CFLAGS) $< -o $%

test: test.cc dmx-eth.o
	$(CC) $(CFLAGS) -c -o test.o test.cc
	$(CC) -lm dmx-eth.o test.o -o test

clean:
	rm test || true
	rm *.o || true
