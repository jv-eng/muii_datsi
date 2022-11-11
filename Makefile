CFLAGS=-Wall

PROGRAMAS= AHCI-version search-tipo-dispo 

all:
	gcc ${CFLAGS} AHCI-version.c -o AHCI-version
	gcc ${CFLAGS} search-tipo-dispo.c -o search-tipo-dispo

clean:
	rm -f $(PROGRAMAS) *.o
