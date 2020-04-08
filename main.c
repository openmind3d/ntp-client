#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <unistd.h>



typedef struct 
{
	uint8_t li_vn_mode;     // Eight bits. li, vn, and mode.
                           	// li.   Two bits.   Leap indicator.
                           	// vn.   Three bits. Version number of the protocol.
                           	// mode. Three bits. Client will pick mode 3 for client.

	uint8_t stratum;         // Eight bits. Stratum level of the local clock.
	uint8_t poll;            // Eight bits. Maximum interval between successive messages.
	uint8_t precision;       // Eight bits. Precision of the local clock.

	uint32_t rootDelay;      // 32 bits. Total round trip delay time.
	uint32_t rootDispersion; // 32 bits. Max error aloud from primary clock source.
	uint32_t refId;          // 32 bits. Reference clock identifier.

	uint32_t refTm_s;        // 32 bits. Reference time-stamp seconds.
	uint32_t refTm_f;        // 32 bits. Reference time-stamp fraction of a second.

	uint32_t origTm_s;       // 32 bits. Originate time-stamp seconds.
	uint32_t origTm_f;       // 32 bits. Originate time-stamp fraction of a second.

	uint32_t rxTm_s;         // 32 bits. Received time-stamp seconds.
	uint32_t rxTm_f;         // 32 bits. Received time-stamp fraction of a second.

	uint32_t txTm_s;         // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
	uint32_t txTm_f;         // 32 bits. Transmit time-stamp fraction of a second.
} ntp_packet;


#define PORTNO 123
#define NTP_TIMESTAMP_DELTA 2208988800ull


int main(int argc, char *argv[])
{
	char hostname[256];

	if(argc < 2)
	{
		printf("required command-line argument <hostname> which is ntp-server\n");
		exit(0);
	}
	else if(argc > 2)
	{
		printf("using only one command-line argument %s\n", argv[1]);
	}
	

	if(strcpy(hostname, argv[1]) == NULL)
	{
		perror("error in copy argv[1] to hostname");
	}
	printf("hostname = %s\n", hostname);


	ntp_packet packet = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	memset(&packet, 0, sizeof(ntp_packet));

	// Set the first byte's bits to 00,011,011 for li = 0, vn = 3, and mode = 3. The rest will be left set to zero.
	*((char *) &packet + 0) = 0x1b;	// Represents 27 in base 10 or 00011011 in base 2.

	struct sockaddr_in server_address;
	struct hostent *server;

	int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sockfd < 0)
	{
		perror("error in creating socket!");
	}



	server = gethostbyname(hostname);	// convert URL to IP-address
	if(server == NULL)
	{
		perror("Host not finded");
	}

	// fill in with zeros
	bzero((char *) &server_address, sizeof(server_address));
	// we using IPv4
	server_address.sin_family = AF_INET;

	// copy host IP-address of host to server_address structure
	bcopy((char *) server->h_addr, (char *) &server_address.sin_addr.s_addr, server->h_length);

	server_address.sin_port = htons(PORTNO);


	int status;
	status = connect(sockfd, (struct sockaddr *) &server_address, sizeof(server_address));
	if(status < 0)
	{
		perror("error with connecting");
	}

	int byte_count;
	byte_count = write(sockfd, (char *) &packet, sizeof(ntp_packet));
	if(byte_count < 0)
	{
		perror("error in writing in socket");
	}

	// reading a message
	byte_count = read(sockfd, (char *) &packet, sizeof(ntp_packet));
	if(byte_count < 0)
	{
		perror("error in reading from socket");
	}

	// parse the return message
	packet.txTm_s = ntohl(packet.txTm_s);
	packet.txTm_f = ntohl(packet.txTm_f);


	time_t txTm = (time_t) (packet.txTm_s - NTP_TIMESTAMP_DELTA);


	printf("Time: %s", ctime((const time_t *) &txTm));

	return 0;
}