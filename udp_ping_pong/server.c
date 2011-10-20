#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUF_SIZE 500

/* This program implements an echo server.
 * The function getaddrinfo is used in conjunction with the specified
 * port to be able to create a localhost datagram socket and bind to it.
 * Using this socket UDP-messages are received with recvfrom, and
 * echoed back using send.
 *
 * Standard functions used (and what for):
 *   getaddrinfo    - Get address info, in this case only datagram
 *                    on a local-host port.
 *   socket         - To create a socket on one of the results from
 *                    getaddrinfo.
 *   bind           - To be able to receive and send from the opened socket.
 *   recvfrom       - Blocking receive from the socket from a peer.
 *                    This function also provides enough information on
 *                    the sender to be able to send back received data.
 *   getnameinfo    - Get name of peer and port and print this.
 *   send           - Used to send back received data to peer.
 */

static int get_addrinfo_on_port(struct addrinfo **result, const char *port)
{
        const char *node = NULL; /* Means loopback interface. */
        struct addrinfo hints;
        int status;

        /* Setup a hint to allow IPv4 or IPv6 with only datagram sockets. */
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC; /* Allow IPv4 or IPv6. */
        hints.ai_socktype = SOCK_DGRAM; /* Datagram socket. */
        hints.ai_flags = AI_PASSIVE; /* For Wildcard IP Address. */
        hints.ai_protocol = 0;
        hints.ai_canonname = NULL;
        hints.ai_addr = NULL;
        hints.ai_next = NULL;

        /* Request address info on localhost, given port and only
         * datagram sockets.
         */
        status = getaddrinfo(node, port, &hints, result);
        if (status != 0)
        {
                fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
                return __LINE__;
        }

        return 0;
}

static int get_bound_socket(struct addrinfo *addrinfo, int *result)
{
        struct addrinfo *curr;
        int sfd;

        for (curr = addrinfo; curr != NULL; curr = curr->ai_next)
        {
                sfd = socket(curr->ai_family, curr->ai_socktype,
                                curr->ai_protocol);
                if (sfd == -1)
                {
                        continue;
                }

                if (bind(sfd, curr->ai_addr, curr->ai_addrlen) == 0)
                {
                        *result = sfd;
                        return 0;
                }

                close(sfd);
        }

        return __LINE__;
}

static void print_name_info(struct sockaddr_storage peer_addr,
                            socklen_t peer_addr_len)
{
        char host[NI_MAXHOST];
        char service[NI_MAXSERV];
        int status;
        
        status = getnameinfo((struct sockaddr *) &peer_addr, peer_addr_len,
                             host, NI_MAXHOST, service, NI_MAXSERV,
                             NI_NUMERICSERV);
        if (status != 0)
        {
                fprintf(stderr, "getnameinfo: %s.\n", gai_strerror(status));
        }
                
        printf("Received from %s:%s.\n", host, service);
}

static int echo_server(int sfd)
{
        struct sockaddr_storage peer_addr;
        char buf[BUF_SIZE];
        socklen_t peer_addr_len = sizeof(struct sockaddr_storage);
        ssize_t nread;
        int status;

        /* Receive from socket. */
        nread = recvfrom(sfd, buf, BUF_SIZE, 0, (struct sockaddr *)&peer_addr,
                         &peer_addr_len);
        if (nread == -1)
        {
                return 0; /* Received nothing. */
        }

        /* Print information about the sending peer. */
        print_name_info(peer_addr, peer_addr_len);
 
        /* Send back the information to the peer. */
        status = sendto(sfd, buf, nread, 0, (struct sockaddr*)&peer_addr,
                        peer_addr_len);
        if (status != nread)
        {
                fprintf(stderr, "sendto: %s.\n", gai_strerror(status));
                exit(__LINE__);
        }

        return 0;
}

int main(int argc, char *argv[])
{
        struct addrinfo *result;
        int sfd;
        int status;
        char *port;

        if (argc != 2)
        {
                fprintf(stderr, "Usage: %s port\n", argv[0]);
                exit(__LINE__);
        }

        /* Get address info on the specified port on localhost. */
        port = argv[1];
        status = get_addrinfo_on_port(&result, port);
        if (status != 0)
        {
                exit(status);
        }

        /* Get a bound socket to one of the addrinfo:s. */
        status = get_bound_socket(result, &sfd);
        if (status != 0)
        {
                exit(status);
        }
        freeaddrinfo(result);

        /* We have now successfully opened a datagram socket, and
         * bind to it, and can start receiving from it.
         */
        for (;;)
        {
                status = echo_server(sfd);
                if (status != 0)
                {
                        exit(status);
                }
        }
}
