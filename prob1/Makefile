CC = gcc
CFLAGS = -Wall
LIBS = -lssl -lcrypto

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

fingerprint: fingerprint.o
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

all: fingerprint

.PHONY: clean

clean:
	rm -f *.o fingerprint
