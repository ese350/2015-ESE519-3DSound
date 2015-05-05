CC	= g++
CFLAGS	= -std=c++0x -Ofast
LFLAGS	= -ljack -lrt

all: simple_client

clean: 
	rm *.o
		
simple_client: simple_client.o complex.o fft.o mit_hrtf_lib.o Mixer3D.o
	$(CC) -o $@ $^ $(LFLAGS)

simple_client.o: simple_client.c *.h
	$(CC) $(CFLAGS) -c $<

complex.o: Mixer/source/complex.cpp
	$(CC) $(CFLAGS) -c $<

fft.o: Mixer/source/fft.cpp
	$(CC) $(CFLAGS) -c $<

mit_hrtf_lib.o: Mixer/source/mit_hrtf_lib.c
	$(CC) $(CFLAGS) -c $<

Mixer3D.o: Mixer/source/Mixer3D.cpp *.h
	$(CC) $(CFLAGS) -c $<
