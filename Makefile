CFLAGS=-g -Wall -fPIC -I/usr/local/include
LIBS=-L/usr/local/lib -lHashTableO1

all: libNgramsLocations

test: testNgramsLocations
	./testNgramsLocations test-map.ngt test-map-out.ngt
	cmp -l test-map.ngt test-map-out.ngt

testNgramsLocations: Makefile ngramsLocations.h testNgramsLocations.c
	gcc ${CFLAGS} -o testNgramsLocations testNgramsLocations.c ${LIBS} -lNgramsLocations

libNgramsLocations: Makefile ngramsLocations.o ngramsLocations.h
	gcc -shared -o libNgramsLocations.so.1.0 ngramsLocations.o ${LIBS}

ngramsLocations.o: Makefile ngramsLocations.h ngramsLocations.c
	gcc ${CFLAGS} -c ngramsLocations.c -o ngramsLocations.o

install:
	cp libNgramsLocations.so.1.0 /usr/local/lib
	ln -sf /usr/local/lib/libNgramsLocations.so.1.0 /usr/local/lib/libNgramsLocations.so.1
	ln -sf /usr/local/lib/libNgramsLocations.so.1.0 /usr/local/lib/libNgramsLocations.so
	ldconfig /usr/local/lib
	cp ngramsLocations.h /usr/local/include/ngramsLocations.h

clean:
	rm *.o; rm *.so*; rm core*; rm testNgramsLocations
