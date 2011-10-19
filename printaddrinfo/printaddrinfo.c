#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "printaddrinfo.h"

#define STR(x) #x

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*(array)))

struct info_elem;

static int compare_flag(uint32_t elem_value, uint32_t ai_value);
static int compare_ls_nibble(uint32_t elem_value, uint32_t ai_value);
static int compare_value(uint32_t elem_value, uint32_t ai_value);

struct info_elem
{
        const uint32_t value;
        const char *name;
        const char *desc;
};

struct info
{
        int (*is_match)(uint32_t elem_value, uint32_t ai_value);
        const size_t num_elems;
        const struct info_elem *info_elems;
};

static const struct info_elem ai_flags_info_elems[] =
{
        {AI_PASSIVE, STR(AI_PASSIVE),
         "Socket address is intended for bind."},
        {AI_CANONNAME, STR(AI_CANONNAME),
         "Request for canonical name."},
        {AI_NUMERICHOST, STR(AI_NUMERICHOST),
         "Don't use name resolution."},
        {AI_V4MAPPED, STR(AI_V4MAPPED),
         "IPv4 mapped addresses are acceptable."},
        {AI_ALL, STR(AI_ALL),
         "Return IPv4 mapped and IPv6 addresses."},
        {AI_ADDRCONFIG, STR(AI_ADDRCONFIG),
         "Use configuration of this host to choose returned address type."},
        {AI_IDN, STR(AI_IDN),
         "IDN encode input (assuming it is encoded in the current "
         "locale's character set) before looking it up."},
        {AI_CANONIDN, STR(AI_CANONIDN),
         "Translate canonical name from IDN format."},
        {AI_IDN_ALLOW_UNASSIGNED, STR(AI_IDN_ALLOW_UNASSIGNED),
         "Don't reject unassigned Unicode code points."},
        {AI_IDN_USE_STD3_ASCII_RULES, STR(AI_IDN_USE_STD3_ASCII_RULES),
         "Validate strings according to STD3 rules."},
};

static const struct info ai_flags_info =
{
        compare_flag,
        ARRAY_SIZE(ai_flags_info_elems),
        ai_flags_info_elems
};

static const struct info_elem ai_family_info_elems[] =
{
        {AF_UNIX, STR(AF_UNIX), "Local communication."},
        {AF_LOCAL, STR(AF_LOCAL), " Local communication."},
        {AF_INET, STR(AF_INET), "IPv4 Internet protocols."},
        {AF_INET6, STR(AF_INET6), "IPv6 Internet protocols."},
        {AF_IPX, STR(AF_IPX), "IPX - Novell protocols."},
        {AF_NETLINK, STR(AF_NETLINK), "Kernel user interface device."},
        {AF_X25, STR(AF_X25), "ITU-T X.25 / ISO-8208 protocol."},
        {AF_AX25, STR(AF_AX25), "Amateur radio AX.25 protocol."},
        {AF_ATMPVC, STR(AF_ATMPVC), "Access to raw ATM PVCs."},
        {AF_APPLETALK, STR(AF_APPLETALK), "Appletalk."},
        {AF_PACKET, STR(AF_PACKET), "Low level packet interface."},
};

static const struct info ai_family_info =
{
        compare_ls_nibble,
        ARRAY_SIZE(ai_family_info_elems),
        ai_family_info_elems
};

static const struct info_elem ai_socktype_info_elems[] =
{
        {SOCK_STREAM, STR(SOCK_STREAM),
         "Sequenced, reliable, connection-based byte streams."},
        {SOCK_DGRAM, STR(SOCK_DGRAM),
         "Connectionless, unreliable datagrams of fixed maximum length."},
        {SOCK_RAW, STR(SOCK_RAW),
         "Raw protocol interface."},
        {SOCK_RDM, STR(SOCK_RDM),
         "Reliably-delivered messages."},
        {SOCK_SEQPACKET, STR(SOCK_SEQPACKET),
         "Sequenced, reliable, connection-based, datagrams of fixed maximum "
         "length."},
        {SOCK_DCCP, STR(SOCK_DCCP),
         "Datagram Congestion Control Protocol."},
        {SOCK_PACKET, STR(SOCK_PACKET),
         "Linux specific way of getting packets at the dev level." 
         "For writing rarp and other similar things on the user level."},
};

static const struct info ai_socktype_info =
{
        compare_ls_nibble,
        ARRAY_SIZE(ai_socktype_info_elems),
        ai_socktype_info_elems
};

static const struct info_elem ai_socktype_flags_info_elems[] =
{
        {SOCK_CLOEXEC, STR(SOCK_CLOEXEC),
         "Atomically set close-on-exec flag for the new descriptor(s)."},
        {SOCK_NONBLOCK, STR(SOCK_NONBLOCK),
         "Atomically mark descriptor(s) as non-blocking."}
};

static const struct info ai_socktype_flags_info =
{
        compare_flag,
        ARRAY_SIZE(ai_socktype_flags_info_elems),
        ai_socktype_flags_info_elems
};

static const struct info_elem errno_info_elems[] =
{
        {EAI_ADDRFAMILY, STR(EAI_ADDRFAMILY),
         "The specified network host does not have any network addresses in "
         "the requested address family."},
        {EAI_AGAIN, STR(EAI_AGAIN),
         "The name server returned a temporary failure indication. Try again "
         "later."},
        {EAI_BADFLAGS, STR(EAI_BADFLAGS),
         "hints.ai_flags contains invalid flags; or, hints.ai_flags "
         "included AI_CANONNAME and name was NULL."},
        {EAI_FAIL, STR(EAI_FAIL),
         "The name server returned a permanent failure indication."},
        {EAI_FAMILY, STR(EAI_FAMILY),
         "The requested address family is not supported."},
        {EAI_MEMORY, STR(EAI_MEMORY),
         "Out of memory."},
        {EAI_NODATA, STR(EAI_NODATA),
         "The specified network host exists, but does not have any network "
         "addresses defined."},
        {EAI_NONAME, STR(EAI_NONAME),
         "The node or service is not known; or both node and service are "
         "NULL; or AI_NUMERICSERV was specified in hints.ai_flags and "
         "service was not a numeric port-number string."},
        {EAI_SERVICE, STR(EAI_SERVICE),
         "The requested service is not available for the requested socket "
         "type. It may be available through another socket type. "
         "For example, this error could occur if service was \"shell\" "
         "(a service only available on stream sockets), and either "
         "hints.ai_protocol was IPPROTO_UDP, or hints.ai_socktype was "
         "SOCK_DGRAM; or the error could occur if service was "
         "not NULL, and hints.ai_socktype was SOCK_RAW (a socket type that "
         "does not support the concept of services)."},
        {EAI_SOCKTYPE, STR(EAI_SOCKTYPE),
         "The requested socket type is not supported. This could "
         "occur, for example, if hints.ai_socktype  and "
         "hints.ai_protocol  are  inconsistent  (e.g., SOCK_DGRAM and "
         "IPPROTO_TCP, respectively)."},
        {EAI_SYSTEM, STR(EAI_SYSTEM),
         "Other system error, check errno for details."}
};

static const struct info errno_info =
{
        compare_value,
        ARRAY_SIZE(errno_info_elems),
        errno_info_elems
};

static int compare_flag(uint32_t elem_value, uint32_t ai_value)
{
        return elem_value & ai_value;
}

static int compare_ls_nibble(uint32_t elem_value, uint32_t ai_value)
{
        return (ai_value & 0x0000000FUL) == elem_value;
}

static int compare_value(uint32_t elem_value, uint32_t ai_value)
{
        return elem_value == ai_value;
}

static size_t get_num_columns(void)
{
        struct winsize w;
        size_t cols;
        
        ioctl(0, TIOCGWINSZ, &w);
        cols = w.ws_col;

        if (cols == 0)
        {
                cols = 60;
        }
        
        return cols;
}

static void
print_description(const char *prefix, const char *description,
                  size_t description_len, int count, size_t prefix_len,
                  size_t num_columns)
{
        if (count == 0)
        {
                printf("%s", prefix);
        }
        else
        {
                char space[100];
                memset(space, ' ', prefix_len);
                space[prefix_len] = '\0';
                printf("%s", space);
        }

        if (*description == ' ')
        {
                description++;
                description_len--;
        }
        
        unsigned int cols_left = num_columns - prefix_len;
        if (cols_left > description_len)
        {
                printf("%s\n", description);
        }
        else
        {
                char line[100];

                strcpy(line, description);
                line[cols_left] = '\0';
                printf("%s\n", line);
                description += cols_left;
                print_description(prefix, description, strlen(description),
                                  count + 1, prefix_len, num_columns);
        }
}

static void
print_info_and_description(const char *msg, const char *name,
                           const char *description)
{
        static size_t num_columns;
        char prefix[100];
        
        if (num_columns == 0)
        {
                num_columns = get_num_columns();
        }
                                
        sprintf(prefix, "%s%-15s ", msg, name);
        print_description(prefix, description, strlen(description), 0,
                          strlen(prefix), num_columns);
}

static void print_info(const char *msg, uint32_t ai_value,
                       const struct info *info, int print_description)
{
        unsigned int i;

        for (i = 0; i < info->num_elems; i++)
        {
                const struct info_elem *elem = &info->info_elems[i];
                
                if (info->is_match(elem->value, ai_value))
                {
                        if (print_description)
                        {
                                print_info_and_description(msg, elem->name,
                                                           elem->desc);
                        }
                        else
                        {
                                printf("%s%s\n", msg, elem->name);
                        }
                }
        }
}

static void
print_failed_status(int status)
{
        char msg[100];
        const int print_description = 1;

        sprintf(msg, "Error: ");
        print_info(msg, status, &errno_info, print_description);
}

static void print_ai_flags(int order, int print_description, int ai_flags)
{
        char msg1[100];
        char msg2[100];

        sprintf(msg1, "[%d]->ai_flags", order);
        sprintf(msg2, "%-20s", msg1);
        print_info(msg2, ai_flags, &ai_flags_info, print_description);
}

static void print_ai_family(int order, int print_description, int ai_family)
{
        char msg1[100];
        char msg2[100];

        sprintf(msg1, "[%d]->ai_family", order);
        sprintf(msg2, "%-20s", msg1);
        print_info(msg2, ai_family, &ai_family_info, print_description);
}

static void print_ai_socktype(int order, int print_description,
                                int ai_socktype)
{
        char msg1[100];
        char msg2[100];

        sprintf(msg1, "[%d]->ai_socktype", order);
        sprintf(msg2, "%-20s", msg1);
        print_info(msg2, ai_socktype, &ai_socktype_info, print_description);
        print_info(msg2, ai_socktype, &ai_socktype_flags_info,
                     print_description);
}

static void print_ai_protocol(int order, int ai_protocol)
{
        char msg1[100];
        char msg2[100];

        sprintf(msg1, "[%d]->ai_protocol", order);
        sprintf(msg2, "%-20s", msg1);

        struct protoent *protoent = getprotobynumber(ai_protocol);
        printf("%s%s\n", msg2, protoent->p_name);
}

static void print_ai_canonname(int order, char *ai_canonname)
{
        if (ai_canonname == NULL)
        {
                return;
        }
        
        char msg1[100];
        char msg2[100];

        sprintf(msg1, "[%d]->ai_canonname", order);
        sprintf(msg2, "%-20s", msg1);
        printf("%s%s\n", msg2, ai_canonname);
}

static void print_ai(const struct addrinfo *addrinfo,
                     int print_description)
{
        static int order = 0;
        
        print_ai_family(order, print_description, addrinfo->ai_family);
        print_ai_flags(order, print_description, addrinfo->ai_flags);
        print_ai_socktype(order, print_description, addrinfo->ai_socktype);
        print_ai_protocol(order, addrinfo->ai_protocol);
        print_ai_canonname(order, addrinfo->ai_canonname);

        if (addrinfo->ai_next != NULL)
        {
                printf("\n");
                order++;
                print_ai(addrinfo->ai_next, print_description);
        }
}

static void
print_title(const char *address)
{
        char msg[100];
        size_t len;
        
        sprintf(msg, "Address information for \"%s\"",
                (address == NULL) ? "localhost:<any-port>" : address);
        len = strlen(msg);
        printf("%s\n", msg);
        memset(msg, '=', len);
        msg[len] = '\0';
        printf("%s\n", msg);
}

static void
get_host_and_port(const char *address, char **host_ptr, char **port_ptr)
{
        if (address == NULL)
        {
                *host_ptr = NULL;
                *port_ptr = NULL;
        }
        else
        {
                char *colon;

                *host_ptr = malloc(40);
                strcpy(*host_ptr, address);
                colon = strstr(*host_ptr, ":");
                if (colon == NULL)
                {
                        *port_ptr = NULL;
                }
                else
                {
                        *colon = '\0';
                        
                        if (*(colon + 1) == '\0')
                        {
                                *port_ptr = NULL;
                        }
                        else
                        {
                                *port_ptr = malloc(40);
                                strcpy(*port_ptr, colon + 1);
                        }
                }
        }
}

static void free_host_and_port(char *host, char *port)
{
        if (host != NULL)
        {
                free(host);

        }
        
        if (port != NULL)
        {
                free(port);
        }
}

void print_addr_info(const char *address, int print_description)
{
        struct addrinfo *res;
        int status;
        char *host;
        char *port;

        print_title(address);
        get_host_and_port(address, &host, &port);
        status = getaddrinfo(host, port, NULL, &res);
        free_host_and_port(host, port);
        if (status != 0)
        {
                print_failed_status(status);
                return;
        }
        
        print_ai(res, print_description);
        freeaddrinfo(res);
}
