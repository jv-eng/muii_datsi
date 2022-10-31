CFLAGS=-Wall

all:
	gcc ${CFLAGS} AHCI-version.c -o AHCI-version
	gcc ${CFLAGS} search-tipo-disco.c -o search-tipo-disco

clean:
	rm -f $(PROGRAMAS) *.o