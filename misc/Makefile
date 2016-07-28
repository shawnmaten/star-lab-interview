BDIR = ./bin
IDIR = ./include
ODIR = ./obj
SDIR = ./src

CC = gcc
CFLAGS = -Wall -I $(IDIR)
LIBS = -lssl -lcrypto

_OBJ = prob1.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.c $(IDIR)/%.h
	$(CC) -c -o $@ $< $(CFLAGS)

prob1: $(ODIR)/prob1.o
	gcc -o $(BDIR)/$@ $^ $(CFLAGS) $(LIBS)

all: prob1

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o $(BDIR)/*
