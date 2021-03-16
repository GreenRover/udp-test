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

#define PORT (3000)

#define SUBIMAGES (1000)
#define FRAMES_PER_SUBIMAGES (250)
#define UDP_FRAME (1464)

#define SERVERADDRESS "239.192.9.9"
#define CLIENT_INTERFACE "10.172.160.251"

#define BUFFER_SIZE (SUBIMAGES * FRAMES_PER_SUBIMAGES * UDP_FRAME)

struct Frame {
    char code[4];
    int32_t counter;
};

#endif /* COMMON_H */
