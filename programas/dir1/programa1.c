#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>

int v=5;
int main(int argc, char *argv[]) {
	++v;
        printf("%d %p %p %p\nPulsa para terminar ", getpid(), main, &v, printf);
        getchar();
	return 0;
}
