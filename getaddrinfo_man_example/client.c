#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 500

int main(int argc, char *argv[])
{
        struct addrinfo hints;
        struct addrinfo *result;
        struct addrinfo *rp;
        int sfd;
        int s;
        size_t len;
        ssize_t nread;
        char buf[BUF_SIZE];

        if (argc < 3)
        {
                fprintf(stderr, "Usage: %s host port msg\n", argv[0]);
                exit(__LINE__);
        }

        /* Get address information on specified host and port. */
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC; /* Allow IPv4 or IPv6. */
        hints.ai_socktype = SOCK_DGRAM; /* Datagram socket. */
        s = getaddrinfo(argv[1], argv[2], &hints, &result);
        if (s != 0)
        {
                fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
                exit(__LINE__);
        }

        /* Go through each addrinfo and try to open a socket an
         * connect to it. Either use the first one or exit.
         */
        for (rp = result; rp != NULL; rp = rp->ai_next)
        {
                sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
                if (sfd == -1)
                {
                        continue;
                }

                s = connect(sfd, rp->ai_addr, rp->ai_addrlen);
                if (s != -1)
                {
                        break;
                }

                close(sfd);
        }

        /* Exit if we could not socket+connect at least once. */
        if (rp == NULL)
        {
                fprintf(stderr, "Could not connect.\n");
                exit(__LINE__);
        }

        freeaddrinfo(result);

        /* Send specified message as a separat datagram and read
         * the response from the server.
         */
        len = strlen(argv[3]) + 1;
        s = write(sfd, argv[3], len);
        if (s != (int)len)
        {
                fprintf(stderr, "Partial write failed.\n");
                exit(__LINE__);
        }

        nread = read(sfd, buf, BUF_SIZE);
        if (nread == -1)
        {
                perror("read");
                exit(__LINE__);
        }

        printf("Received %ld bytes: \"%s\"\n", (long)nread, buf);

        return 0;
}
