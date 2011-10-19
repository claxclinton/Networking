#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUF_SIZE 500

int
main(int argc, char *argv[])
{
        struct addrinfo hints;
        struct addrinfo *result, *rp;
        int sfd, s;
        struct sockaddr_storage peer_addr;
        char buf[BUF_SIZE];
        char *port;

        if (argc != 2)
        {
                fprintf(stderr, "Usage: %s port\n", argv[0]);
                exit(__LINE__);
        }
        port = argv[1];

        /* Setup a hint to allow IPv4 or IPv6 with only datagram
           sockets. */
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC; /* Allow IPv4 or IPv6. */
        hints.ai_socktype = SOCK_DGRAM; /* Datagram socket. */
        hints.ai_flags = AI_PASSIVE; /* For Wildcard IP Address. */
        hints.ai_protocol = 0;
        hints.ai_canonname = NULL;
        hints.ai_addr = NULL;
        hints.ai_next = NULL;

        s = getaddrinfo(NULL, port, &hints, &result);
        if (s != 0)
        {
                fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
                exit(__LINE__);
        }

        /* getaddrinfo() returns a list of address structures.
         * Try each address until we successfully bind(2).
         * If socket(2) (or bind(2)) fails, we (close the socket)
         * try the next address.
         */
        for (rp = result; rp != NULL; rp = rp->ai_next)
        {
                sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

                if (sfd == -1)
                {
                        continue; /* Failed. */
                }

                if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
                {
                        break; /* Succeeded. */
                }

                close(sfd);
        }

        if (rp == NULL)
        {
                fprintf(stderr, "Could not bind.\n");
                exit(__LINE__);
        }

        freeaddrinfo(result);

        /* We have now successfully opened a datagram socket, and
         * bind to it, and can start receiving from it.
         */
        for (;;)
        {
                socklen_t peer_addr_len = sizeof(struct sockaddr_storage);
                ssize_t nread;

                nread = recvfrom(sfd, buf, BUF_SIZE, 0,
                                 (struct sockaddr *)&peer_addr, &peer_addr_len);
                if (nread == -1)
                {
                        continue; /* Received nothing. */
                }

                /* Get information about the sending peer. */
                char host[NI_MAXHOST];
                char service[NI_MAXSERV];
                s = getnameinfo((struct sockaddr *) &peer_addr,
                                peer_addr_len, host, NI_MAXHOST,
                                service, NI_MAXSERV, NI_NUMERICSERV);
                if (s != 0)
                {
                        fprintf(stderr, "getnameinfo: %s.\n", gai_strerror(s));
                        exit(__LINE__);
                }
                
                printf("Received %ld bytes from %s:%s.\n", (long)nread, host,
                       service);

                /* Send back the information to the peer. */
                s = sendto(sfd, buf, nread, 0, (struct sockaddr*)&peer_addr,
                           peer_addr_len);
                if (s != nread)
                {
                        fprintf(stderr, "sendto: %s.\n", gai_strerror(s));
                        exit(__LINE__);
                }
        }
}
        
