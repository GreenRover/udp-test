/**
 * With strace reverse engineered the problem of rvrd.
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
    struct timespec startTs, endTs;
    int sockfd;
    struct sockaddr_in server;

    char* CLIENT_INTERFACE = argv[1];

    printf("Using interface %d\n", CLIENT_INTERFACE);
    printf("Build Data...\n");
    build(buffer, sizeof(buffer));

    printf("Configure socket...\n");
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

    if (sockfd < 0)
    {
        perror("Error opening socket");
        return EXIT_FAILURE;
    }

  // 	int sndbuf = 25165824;      /* getsockopt(13, SOL_SOCKET, SO_SNDBUF, [25165824], [4]) = 0 */

	/* setsockopt(13, SOL_IP, 0x31 IP_??? , [0], 4) = 0 */
     int xxx = 0;
     if (setsockopt(sockfd, SOL_IP, 0x31, &xxx, sizeof(xxx)) < 0)
     {
        perror("xxx");
        return EXIT_FAILURE;
     }

	/* bind(13, {sa_family=AF_INET, sin_port=htons(0), sin_addr=inet_addr("0.0.0.0")}, 16) = 0 */
	struct sockaddr_in si_me;
    bzero(&(si_me.sin_zero),8);
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(0);
    si_me.sin_addr.s_addr = INADDR_ANY;

    printf("Bind socket...\n");
    if (bind(sockfd,(struct sockaddr *)&si_me, sizeof(si_me)) == -1)
    {
        printf("Error in bind(): %d\n", errno);
        return EXIT_FAILURE;
    }

	/* setsockopt(13, SOL_IP, IP_MULTICAST_LOOP, "\0", 1) = 0 */
	/* dont receive self what i send */
  	int multicastLoop = 0;
  	if (setsockopt(sockfd, SOL_IP, IP_MULTICAST_LOOP, &multicastLoop, sizeof(multicastLoop)) < 0)
    {
     	perror("IP_MULTICAST_LOOP");
        return EXIT_FAILURE;
    }

	/* setsockopt(13, SOL_IP, IP_MULTICAST_TTL, "\20", 1) = 0 */
    int ttl;
    ttl = 20;
    if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0)
    {
    	perror("IP_MULTICAST_TTL");
    	return EXIT_FAILURE;
    }

    if (fcntl(sockfd, F_SETFL, O_RDWR|O_NONBLOCK) < 0)
    {
    	perror("F_SETFL");
        return EXIT_FAILURE;
    }

	/* setsockopt(13, SOL_IP, IP_MULTICAST_IF, [-56579062], 4) = 0 */
	struct in_addr localInterface;
	localInterface.s_addr = inet_addr(CLIENT_INTERFACE);
	if (setsockopt(sockfd, SOL_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0)
    {
    	perror("IP_MULTICAST_IF");
        return EXIT_FAILURE;
    }

	/* setsockopt(13, SOL_SOCKET, SO_BROADCAST, [1], 4) = 0 */
   	int broadcastEnable=1;
   	if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) < 0)
    {
       	perror("SO_BROADCAST");
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

    printf("Send UDP data...\n");

	setlocale( LC_ALL, "de_CH.UTF-8" );

    size_t x;
    for (x = 0; x < 30; x++) {
    	clock_gettime(CLOCK_MONOTONIC_RAW, &startTs);

		size_t i;
		for (i = 0; i < BUFFER_SIZE; )
		{
			memcpy(&buffer[i], "MILA", 4);
			*(int32_t*)(&buffer[i + 4]) = i;
			size_t transmittedBytes = sendto(
				sockfd,
				&buffer[i],
				UDP_FRAME,
				0,
				(const struct sockaddr*)&server,
				sizeof(server)
			  );
			if (transmittedBytes < 0)
			{
				fprintf(stderr, "Error in sendto()\n");
				return EXIT_FAILURE;
			}

			i += transmittedBytes;
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
