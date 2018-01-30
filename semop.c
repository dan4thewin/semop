/* Licensed under Apache License v2.0 - see LICENSE file for details */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>

/* according to X/OPEN we have to define this ourselves */
#ifndef __APPLE__
union semun {
    int               val; /* value for SETVAL */
    struct semid_ds  *buf; /* buffer for IPC_STAT, IPC_SET */
    unsigned short *array; /* array for GETALL, SETALL */
    struct seminfo *__buf; /* buffer for IPC_INFO (Linux specific) */
};
#endif

#define NSEMS 1
union semun arg;
unsigned short semval[NSEMS];
static struct sembuf decr[] = {
    { 0, -1, 0 }
};
static struct sembuf incr[] = {
    { 0, 1, 0 }
};
static struct sembuf zero[] = {
    { 0, 0, 0 }
};

key_t a2key(const char *);

char *me;

void usage() {
    fprintf(stderr,
            "Usage: %s [OPTION] <semaphore ascii key>\n"
            "  -i increment\n"
            "  -d decrement, will wait\n"
            "  -D decrement, will not wait (exits 1 on EAGAIN)\n"
            "  -1 set to 1\n"
            "  -z set to 0\n"
            "  -0 return when 0, will wait\n"
            "  -x delete semaphore set\n"
            "  -p print current value\n"
            "  -n print current value only if set exists\n\n"
            "Usage: %s -s <int> <semaphore ascii key>\n"
            "  -s  set to <int>\n\n"
            "If no option is given, -p is assumed.\n"
            "key is 1 to 4 ascii characters to be converted "
            "to a numeric id.\n\n",
            me, me);
    exit(1);
}

void err(const int exitval, const char *msg) {
    fprintf(stderr, errno ? "%s: %s: %s\n" : "%s: %s%s\n",
            me, msg, errno ? strerror(errno) : "");
     exit(exitval);
}
#define errx(x, y) (errno = 0, err(x, y))

#define ok(x) ({                            \
    int __r;                                \
    do {                                    \
        __r = (x);                          \
        if (__r == -1) {                    \
            if (errno == EAGAIN)            \
                exit(1);                    \
            if (errno != EINTR)             \
                err(1, #x);                 \
        }                                   \
    } while (__r == -1);                    \
    __r;                                    \
})

int main (int argc, char **argv) {
    char *p = argv[1];
    key_t key;
    int semid = 0;
    unsigned short setall[] = { 0 };

    me = argv[0];

    switch (argc) {
        case 2:
            if (p[0] == '-')
                usage();
            p = "-p";
            break;
        case 3:     /* all options but -s */
            if (p[0] != '-' || p[1] == 's' || p[2])
                usage();
            break;
        case 4:
            if (p[0] != '-' || p[1] != 's' || p[2])
                usage();
            break;
        default:
            usage();
    }
    p++;

    key = a2key(argv[argc-1]);
    if (key == -1)
        errx(1, "Semaphore ascii key must be one to four characters");

    semid = semget(key, NSEMS, (*p == 'n' ? 0 : IPC_CREAT)|S_IRWXU);
    if (semid == -1) {
        if (*p == 'n' && errno == ENOENT)
            exit(1);
        err(1, "semget");
    }

    switch (*p) {
        case '0':
            ok(semop(semid, zero, 1));
            break;

        case 's':
        case '1':
            setall[0] = *p == 's' ? atoi(argv[2]) : 1;

        case 'z':
            ok(semctl(semid, 0, SETALL, setall));
            break;

        case 'x':
            ok(semctl(semid, 0, IPC_RMID, NULL));
            break;

        case 'D':
            decr[0].sem_flg = IPC_NOWAIT;
        case 'i':
        case 'd':
            ok(semop(semid, *p == 'i' ? incr : decr, 1));

        case 'p':
        case 'n':
            arg.array = semval;
            ok(semctl(semid, 0, GETALL, arg));
            printf("%hu\n", semval[0]);
            break;

        default:
            usage();
    }

    exit(0);
}

/*
 * number of bytes to use should match sizeof(key_t) and sizeof(int)
 */
#define B 4
typedef union {
    char _char[B];
    int _int;
} bytes;

key_t a2key (const char *str) {
    bytes key;
    int i, j;

    assert(sizeof(int) == B);

    assert(sizeof(key_t) == B);

    if (str == NULL || *str == '\0')
        return -1;

    key._int=0;
    for(i = 0; str[i] && i < B; i++);
    for(j = 0, i--; i >= 0; i--, j++)
        key._char[j] = str[i];
    return key._int;
}
