#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct mens {
    long tipo;
    char m[13];
} m;
int main(int argc, char *argv[]) {
    key_t key;
    int   qid;

    key = ftok("/", 'X');

    if((qid = msgget(key, IPC_CREAT|0666)) == -1) {
        perror("msgget");
        exit(1);
    }
    msgrcv(qid, (struct mens *)&m, 13, 0, 0);
    printf("%s", m.m);
    msgctl(qid, IPC_RMID, 0);
    return 0;
}
