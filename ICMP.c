// icmp.cpp
// Robert Iakobashvili for Ariel uni, license BSD/MIT/Apache
// 
// Sending ICMP Echo Requests using Raw-sockets.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h> // gettimeofday()

// ICMP header len for echo req
#define ICMP_HDRLEN 8 

// Checksum algo
unsigned short calculate_checksum(unsigned short * paddress, int len);

#define SOURCE_IP "10.0.2.15"
// i.e the gateway or ping to google.com for their ip-address
#define DESTINATION_IP "10.0.2.4"

int main ()
{
    struct icmp icmphdr; // ICMP-header
    char data[IP_MAXPACKET] = "This is the ping.\n";
    int datalen = strlen(data) + 1;

    //===================
    // ICMP header
    //===================

    // Message Type (8 bits): ICMP_ECHO_REQUEST
    icmphdr.icmp_type = ICMP_ECHO;

    // Message Code (8 bits): echo request
    icmphdr.icmp_code = 0;

    // Identifier (16 bits): some number to trace the response.
    // It will be copied to the response packet and used to map response to the request sent earlier.
    // Thus, it serves as a Transaction-ID when we need to make "ping"
    icmphdr.icmp_id = 18; // hai

    // Sequence Number (16 bits): starts at 0
    icmphdr.icmp_seq = 0;

    // ICMP header checksum (16 bits): set to 0 not to include into checksum calculation
    icmphdr.icmp_cksum = 0;

    // Combine the packet 
    char packet[IP_MAXPACKET];

    // Next, ICMP header
    memcpy ((packet), &icmphdr, ICMP_HDRLEN);

    // After ICMP header, add the ICMP data.
    memcpy (packet+ ICMP_HDRLEN, data, datalen);

    // Calculate the ICMP header checksum
    icmphdr.icmp_cksum = calculate_checksum((unsigned short *) (packet ), ICMP_HDRLEN + datalen);
    memcpy ((packet ), &icmphdr, ICMP_HDRLEN);

    struct sockaddr_in dest_in;
    memset (&dest_in, 0, sizeof (struct sockaddr_in));
    dest_in.sin_family = AF_INET;
    dest_in.sin_addr.s_addr = inet_addr(DESTINATION_IP);

    // Create raw socket for IP-RAW (make IP-header by yourself)
    int sock = -1;
    if ((sock = socket (AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1) 
    {
        fprintf (stderr, "socket() failed with error: %d", errno);
        fprintf (stderr, "To create a raw socket, the process needs to be run by Admin/root user.\n\n");
        return -1;
    }

printf("socket is created successfully\n");

struct timeval start, end;

printf("start sending the ping message..\n");
sleep(2);
gettimeofday(&start, NULL);
    // Send the packet using sendto() for sending datagrams.
    if (sendto (sock, packet, ICMP_HDRLEN + datalen, 0, (struct sockaddr *) &dest_in, sizeof (dest_in)) == -1)  
    {
        fprintf (stderr, "sendto() failed with error: %d", errno);
        return -1;
    }
    printf("sent packet is done successfully\n");
    if (recvfrom (sock, packet, ICMP_HDRLEN + datalen, 0, NULL,NULL) == -1)  
{
        fprintf (stderr, "recive() failed with error: %d" , errno);
        return -1;
    }
gettimeofday(&end, NULL);
float rrt=((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
printf("Time taken for sent ping message is : %.2f micro seconds\n",rrt);

  close(sock);
    printf("close the socket\n");

  return 0;
}

// Compute checksum (RFC 1071).
unsigned short calculate_checksum(unsigned short * paddress, int len)
{
	int nleft = len;
	int sum = 0;
	unsigned short * w = paddress;
	unsigned short answer = 0;

	while (nleft > 1)
	{
		sum += *w++;
		nleft -= 2;
	}

	if (nleft == 1)
	{
		*((unsigned char *)&answer) = *((unsigned char *)w);
		sum += answer;
	}

	// add back carry outs from top 16 bits to low 16 bits
	sum = (sum >> 16) + (sum & 0xffff); // add hi 16 to low 16
	sum += (sum >> 16);                 // add carry
	answer = ~sum;                      // truncate to 16 bits

	return answer;
}

