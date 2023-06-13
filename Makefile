CFLAGS=-Wall
LDLIBS=-lcap

all: servidor1 servidor2 servidor3

servidor3: LDLIBS=-lcap

clean:
	rm -f servidor1 servidor2 servidor3 *.o
