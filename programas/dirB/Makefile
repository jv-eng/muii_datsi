CFLAGS=-Wall -g -fPIC

all: inter.so

inter.so: inter.o
	$(CC) -shared -o $@ $^ -ldl

clean:
	rm -f *.o *.so *.html connect.log

