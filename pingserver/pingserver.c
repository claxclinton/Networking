/* This file implements an ICMP server.
 * This is one by opening two sockets, one for listening and nother
 * one for sending the ICMP reply. The in_cksum function has been copied.
 * The two sockets:
 *   - Receiving on sock_eth with type SOCK_PACKETS which listens to
 *     all network traffic (ETH_P_ALL).
 *   - Sending on sock_icmp which is a SOCK_RAW socket with ICMP as
 *     protocol. setsockopt() is used to allow the socket access to
 *     the headers.
 *   - 
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

#include "pingserver.h"

#define ICMP_TYPE_REPLY 0
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

void pingserver(void)
{
        int one = 1;
        int sock_eth;
        int sock_icmp;
        int status;
        struct sockaddr_in dst;
        struct ip *ip_hdr_in;
        struct ip *ip_hdr_out;
        char buf_in[MAX_MTU];
        char buf_out[MAX_MTU];
        struct icmp *icmp_hdr_in;
        struct icmp *icmp_hdr_out;
        int ip_len;
        int icmp_len;
        int icmp_data_len;

        sock_eth = socket(AF_INET, SOCK_PACKET, htons(ETH_P_ALL));
        if (sock_eth < 0)
        {
                perror("socket");
                exit(__LINE__);
        }
        
        sock_icmp = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
        if (sock_eth < 0)
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

        ip_hdr_in = (struct ip *)(buf_in + sizeof(struct ether_header));
        icmp_hdr_in = (struct icmp *)((unsigned char *)ip_hdr_in +
                                      sizeof(struct ip));
        ip_hdr_out = (struct ip *)buf_out;
        icmp_hdr_out = (struct icmp *)(buf_out + sizeof(struct ip));

        while (1)
        {
                status = recv(sock_eth, buf_in, sizeof(buf_in), 0);
                if (status < 0)
                {
                        perror("recv");
                        exit(__LINE__);
                }

                if (!(ip_hdr_in->ip_p == IPPROTO_ICMP &&
                      icmp_hdr_in->icmp_type == ICMP_ECHO))
                {
                        continue;
                }

                /* Prepare outgoing IP header. */
                ip_hdr_out->ip_v = ip_hdr_in->ip_v;
                ip_hdr_out->ip_hl = ip_hdr_in->ip_hl;
                ip_hdr_out->ip_tos = ip_hdr_in->ip_tos;
                ip_hdr_out->ip_len = ip_hdr_in->ip_len;
                ip_hdr_out->ip_id = ip_hdr_in->ip_id;
                ip_hdr_out->ip_off = 0;
                ip_hdr_out->ip_ttl = 255;
                ip_hdr_out->ip_p = IPPROTO_ICMP;
                ip_hdr_out->ip_sum = 0;
                ip_hdr_out->ip_src.s_addr = ip_hdr_in->ip_dst.s_addr;
                ip_hdr_out->ip_dst.s_addr = ip_hdr_in->ip_src.s_addr;
                ip_hdr_out->ip_sum = in_cksum((unsigned short *)buf_out,
                                               ip_hdr_out->ip_hl);

                /* Prepare outgoing ICMP header. */
                icmp_hdr_out->icmp_type = ICMP_TYPE_REPLY;
                icmp_hdr_out->icmp_code = 0;
                icmp_hdr_out->icmp_cksum = 0;
                icmp_hdr_out->icmp_id = icmp_hdr_in->icmp_id;
                icmp_hdr_out->icmp_seq = icmp_hdr_in->icmp_seq;

                ip_len = ntohs(ip_hdr_out->ip_len);
                icmp_len = ip_len - sizeof(struct iphdr);
                icmp_data_len = icmp_len - sizeof(struct icmphdr);

                printf("ICMP_ECHO request.\n");

                /* Copy ICMP data from request to the reply packet. */
                memcpy(icmp_hdr_out->icmp_data, icmp_hdr_in->icmp_data,
                       icmp_data_len);

                icmp_hdr_out->icmp_cksum = in_cksum((unsigned short *) \
                                                    icmp_hdr_out, icmp_len);
                bzero(&dst, sizeof(dst));
                dst.sin_family = AF_INET;
                dst.sin_addr.s_addr = ip_hdr_out->ip_dst.s_addr;

                status = sendto(sock_icmp, buf_out, ip_len, 0,
                                (struct sockaddr *)&dst, sizeof(dst));
                if (status != ip_len)
                {
                        perror("sendto");
                        exit(__LINE__);
                }
        }
}

