#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 500

/* This example looks for address info of a specified host:port that
 * is of type datagram, and tries to open a socket to this, and
 * connects to it. The call to connect does not mean that the host can
 * be used to send data to it. This guarantee is given when searching
 * for datagram addrinfo.
 *
 * When a socket has been opened, the specified message is sent to
 * this using write and read.
 *
 */
static int
get_server_addr_info(const char *server, const char *port,
                     struct addrinfo **result)
{
        struct addrinfo hints;
        int status;

        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC; /* Allow IPv4 and IPv6. */
        hints.ai_socktype = SOCK_DGRAM; /* Datagram socket. */
        status = getaddrinfo(server, port, &hints, result);
        if (status != 0)
        {
                fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
                return __LINE__;
        }

        return 0;
 }

static int get_connected_socket(struct addrinfo *addrinfo, int *result)
{
        int sfd;
        struct addrinfo *curr;
        
        for (curr = addrinfo; curr != NULL; curr = curr->ai_next)
        {
                sfd = socket(curr->ai_family, curr->ai_socktype,
                             curr->ai_protocol);
                if (sfd == -1)
                {
                        continue;
                }

                int status = connect(sfd, curr->ai_addr, curr->ai_addrlen);
                if (status != -1)
                {
                        break;
                }

                close(sfd);
        }

        if (curr == NULL)
        {
                return __LINE__;
        }

        *result = sfd;

        return 0;
}

static int echo_client(int sfd, const char *msg)
{
        char buf[BUF_SIZE];
        size_t len;
        int status;
        ssize_t nreceived;
        
        len = strlen(msg) + 1;
        printf("Sending %ld bytes: \"%s\"\n", (long)len, msg);
        status = send(sfd, msg, len, 0);
        if (status != (int)len)
        {
                fprintf(stderr, "Partial write failed.\n");
                return __LINE__;
        }

        nreceived = recv(sfd, buf, BUF_SIZE, 0);
        if (nreceived == -1)
        {
                perror("read");
                return __LINE__;
        }

        printf("Received %ld bytes: \"%s\"\n", (long)nreceived, buf);

        return 0;
}

int main(int argc, char *argv[])
{
        struct addrinfo *result;
        int sfd;
        int status;
        const char *host;
        const char *port;
        const char *msg;

        if (argc < 3)
        {
                fprintf(stderr, "Usage: %s host port msg\n", argv[0]);
                exit(__LINE__);
        }

        host = argv[1];
        port = argv[2];
        msg = argv[3];
        
        /* Get address information on specified host and port. */
        status = get_server_addr_info(host, port, &result);
        if (status != 0)
        {
                exit(status);
        }

        /* Go through each addrinfo and try to open a socket an
         * connect to it. Either use the first one or exit.
         */
        status = get_connected_socket(result, &sfd);
        if (status != 0)
        {
                exit(status);
        }
        freeaddrinfo(result);

        /* Send specified message as a separat datagram and read
         * the response from the server.
         */
        status = echo_client(sfd, msg);
        if (status != 0)
        {
                exit(status);
        }

        return 0;
}
