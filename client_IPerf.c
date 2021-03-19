/**
 * With strace reverse engineered the problem of iperf2.0.8
 */
#include "common.h"

char buffer[BUFFER_SIZE];

/**
 * Populate the buffer with random data.
 */
void build(uint8_t* buffer, size_t length)
{
	size_t i;
    for (i = 0; i < length; i++)
    {
        buffer[i] = (rand() % 255) + 1;
    }
}

int main(int argc, char **argv)
{
    struct timespec startTs, endTs, chunkStartTs, chunkEndTs;
    int sockfd;
    struct sockaddr_in server;

    long BANDWIDTH_MBS = strtol(argv[2], NULL, 10);
    char* CLIENT_INTERFACE = argv[1];

    printf("Build Data...\n");
    build(buffer, sizeof(buffer));

    printf("Configure socket...\n");
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

    if (sockfd < 0)
    {
        perror("Error opening socket");
        return EXIT_FAILURE;
    }


	/* setsockopt(3, SOL_IP, IP_MULTICAST_TTL, [16], 4) = 0 */
    int ttl;
    ttl = 16;
    if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0)
    {
  	  perror("IP_MULTICAST_TTL");
      return EXIT_FAILURE;
    }

    bzero((char*)&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(SERVERADDRESS);
    server.sin_port = htons(PORT);

	if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		perror("Error: Connect Failed");
		exit(0);
	}

    struct timeval timeout;
    double intpart, fractpart;
    double sosndtimer;
    sosndtimer = 16;
    fractpart = modf(sosndtimer, &intpart);
    timeout.tv_sec = (int) (intpart);
    timeout.tv_usec = (int) (fractpart * 1e6);
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0 ) {
        perror("SO_SNDTIMEO");
        exit(0);
    }

    printf("Send UDP data...\n");

	setlocale( LC_ALL, "de_CH.UTF-8" );

    size_t x;
    for (x = 0; x < 30; x++) {
    	clock_gettime(CLOCK_MONOTONIC_RAW, &startTs);
    	clock_gettime(CLOCK_MONOTONIC_RAW, &chunkStartTs);

		size_t i;
		size_t chuckSize = 0;
        int soSndbufFilledTimes = 0;
		for (i = 0; i < BUFFER_SIZE; )
		{
			memcpy(&buffer[i], "MILA", 4);
			*(int32_t*)(&buffer[i + 4]) = i;
			size_t transmittedBytes = write(
				sockfd,
				&buffer[i],
				UDP_FRAME
			  );
			if (transmittedBytes < 0)
			{
				fprintf(stderr, "Error in sendto()\n");
				return EXIT_FAILURE;
			}

			i += transmittedBytes;
            chuckSize += transmittedBytes;

            if ((chuckSize + UDP_FRAME) > SO_SNDBUF_SIZE) {
                // SO_SNDBUFF was possible filled
                soSndbufFilledTimes++;

                clock_gettime(CLOCK_MONOTONIC_RAW, &chunkEndTs);
                double chunkDeltaUs = ((chunkEndTs.tv_sec - chunkStartTs.tv_sec) * 1000000.0) +
                                      ((chunkEndTs.tv_nsec - chunkStartTs.tv_nsec) / 1000.0);


                double perBufferChunkUs = 1e6 * (SO_SNDBUF_SIZE * 1.0  / (1.0 * 1e6 * BANDWIDTH_MBS / 8.0));
                // printf("%d: %.2f > %.2f\n", soSndbufFilledTimes, perBufferChunkUs, chunkDeltaUs);

                // Check if we put data into SO_SNDBUF_SIZE faster thant the os /
                // network card is able to send data over wire.
                if (perBufferChunkUs > chunkDeltaUs) {
                    mini_sleep(perBufferChunkUs - chunkDeltaUs);
                }

                chuckSize = 0;
                clock_gettime(CLOCK_MONOTONIC_RAW, &chunkStartTs);
            }
		}

		clock_gettime(CLOCK_MONOTONIC_RAW, &endTs);
		uint64_t delta_us = (endTs.tv_sec - startTs.tv_sec) * 1000000 +
        	(endTs.tv_nsec - startTs.tv_nsec) / 1000;

        int kb = (BUFFER_SIZE / 1000);
        double sec =  ((delta_us * 1.0) / 1e6f);
        printf("chunk %02d | %'d kB | %.3f sec | %'10.0f kbit/s\n", x, kb, sec, kb / sec * 8);
    }
    printf("Finished...\n");

    return EXIT_SUCCESS;
}
