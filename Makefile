CC = clang++
LIBS = -lfftw3 -ljack -lm
INC = -I /Library/Frameworks/Jackmp.framework/Versions/Current/Headers/
CFLAGS = -O3
TARGETS = fft.o dmx-eth.o

all: test

test: test.cc dmx-eth.o
	$(CC) $(CFLAGS) -c -o test.o test.cc
	$(CC) dmx-eth.o test.o -o test

fft: fft.cc dmx-eth.o
	$(CC) $(INC) $(CFLAGS) -c -o fft.o fft.cc
	$(CC) $(LIBS) -lm dmx-eth.o fft.o -o fftw

clean:
	rm fftw || true
	rm test || true
	rm *.o || true
