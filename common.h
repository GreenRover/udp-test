#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <time.h>
#include <errno.h>
#include <locale.h>
#include <math.h>

#define PORT (3000)

#define SUBIMAGES (1000)
#define FRAMES_PER_SUBIMAGES (250)
#define UDP_FRAME (1464)

#define SERVERADDRESS "239.192.9.6"

#define BUFFER_SIZE (SUBIMAGES * FRAMES_PER_SUBIMAGES * UDP_FRAME)

/* getsockopt(3, SOL_SOCKET, SO_SNDBUF, [34000000]> */
#define SO_SNDBUF_SIZE 34000000

void mini_sleep(unsigned long usec) {
    struct timespec requested, remaining;
    requested.tv_sec  = 0;
    requested.tv_nsec = usec * 1000;
    // Note, signals will cause the nanosleep
    // to return early.  That's fine.
    nanosleep(&requested, &remaining);
}

struct Frame {
    char code[4];
    int32_t counter;
};

#endif /* COMMON_H */
