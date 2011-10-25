/* This is one half of an ICMP ping for localhost only.
 * Only q request is sent, the reply is not checked.
 */

#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "pingclient.h"

#define ICMP_TYPE_REPLY 0
#define ICMP_TYPE_REQUEST 8
#define MAX_MTU 1500

/* From Stevens, UNP2ev1 */
unsigned short
in_cksum(unsigned short *addr, int len)
{
    int nleft = len;
    int sum = 0;
    unsigned short *w = addr;
    unsigned short answer = 0;

    while (nleft > 1) {
        sum += *w++;
        nleft -= 2;
    }

    if (nleft == 1) {
        *(unsigned char *)(&answer) = *(unsigned char *)w;
        sum += answer;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;
    return (answer);
}

void pingclient(void)
{
        int one = 1;
        int sock_icmp;
        int status;
        struct ip *ip_hdr_out;
        char buf_out[MAX_MTU];
        struct icmp *icmp_hdr_out;
        int ip_len;
        int icmp_len;
        struct sockaddr_in localhost;

        sock_icmp = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
        if (sock_icmp < 0)
        {
                perror("socket");
                exit(__LINE__);
        }
        
        status = setsockopt(sock_icmp, IPPROTO_IP, IP_HDRINCL,
                            (char *)&one, sizeof(one));
        if (status < 0)
        {
                perror("setsockopt");
                exit(__LINE__);
        }

        ip_hdr_out = (struct ip *)buf_out;
        icmp_hdr_out = (struct icmp *)(buf_out + sizeof(struct ip));

        localhost.sin_family = AF_INET;
        status = inet_pton(AF_INET, "127.0.0.1", &localhost.sin_addr);
        if (status != 1)
        {
                perror("inet_pton");
                exit(__LINE__);
        }

        status = connect(sock_icmp, (const struct sockaddr*)&localhost,
                         sizeof(localhost));
        if (status != 0)
        {
                perror("connect");
                exit(__LINE__);
        }

        /* Prepare outgoing IP header. */
        ip_hdr_out->ip_v = 4;
        ip_hdr_out->ip_hl = 5;
        ip_hdr_out->ip_tos = 0;
        ip_hdr_out->ip_len = 84;
        ip_hdr_out->ip_id = 0;
        ip_hdr_out->ip_off = 0;
        ip_hdr_out->ip_ttl = 64;
        ip_hdr_out->ip_p = IPPROTO_ICMP;
        ip_hdr_out->ip_sum = 0;
        ip_hdr_out->ip_src.s_addr = localhost.sin_addr.s_addr;
        ip_hdr_out->ip_dst.s_addr = localhost.sin_addr.s_addr;
        ip_hdr_out->ip_sum = in_cksum((unsigned short *)buf_out,
                                      ip_hdr_out->ip_hl);

        /* Prepare outgoing ICMP header. */
        icmp_hdr_out->icmp_type = ICMP_TYPE_REQUEST;
        icmp_hdr_out->icmp_code = 0;
        icmp_hdr_out->icmp_cksum = 0;
        icmp_hdr_out->icmp_id = 0x0C5B;
        icmp_hdr_out->icmp_seq = 0x0001;
        ip_len = ip_hdr_out->ip_len;
        icmp_len = ip_len - sizeof(struct iphdr);
        icmp_hdr_out->icmp_cksum = in_cksum((unsigned short *)icmp_hdr_out,
                                            icmp_len);
        
        /* Send packet. */
        status = send(sock_icmp, buf_out, 84, 0);
        if (status != 0)
        {
                perror("send");
                exit(__LINE__);
        }
}

