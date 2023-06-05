CFLAGS=-Wall -g
CFLAGS2=-Wall -g -fPIC

all: programa10

biblio.o: biblio.c
	$(CC) -c $(CFLAGS2) $^

biblio.so: biblio.o
	$(CC) -shared -o $@ $^

programa10: programa10.o ./biblio.so
	$(CC) -no-pie -o $@ $< ./biblio.so

clean:
	rm -f *.o programa10 *.so

