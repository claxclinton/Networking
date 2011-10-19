#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>

#include "printaddrinfo.h"

#define STR(x) #x

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*(array)))

enum info_comparison
{
        INFO_COMPARISON_FLAG = 1,
        INFO_COMPARISON_VALUE = 2,
};

struct info
{
        uint32_t flag;
        const char *name;
        const char *desc;
};

static const struct info ai_flags_info[] =
{
        {AI_PASSIVE, STR(AI_PASSIVE),
         "Socket address is intended for bind"},
        {AI_CANONNAME, STR(AI_CANONNAME),
         "Request for canonical name"},
        {AI_NUMERICHOST, STR(AI_NUMERICHOST),
         "Don't use name resolution"},
        {AI_V4MAPPED, STR(AI_V4MAPPED),
         "IPv4 mapped addresses are acceptable"},
        {AI_ALL, STR(AI_ALL),
         "Return IPv4 mapped and IPv6 addresses"},
        {AI_ADDRCONFIG, STR(AI_ADDRCONFIG),
         "Use configuration of this host to choose returned address type"},
        {AI_IDN, STR(AI_IDN),
         "IDN encode input (assuming it is encoded in the current "
         "locale's character set) before looking it up. "},
        {AI_CANONIDN, STR(AI_CANONIDN),
         "Translate canonical name from IDN format. "},
        {AI_IDN_ALLOW_UNASSIGNED, STR(AI_IDN_ALLOW_UNASSIGNED),
         "Don't reject unassigned Unicode code points"},
        {AI_IDN_USE_STD3_ASCII_RULES, STR(AI_IDN_USE_STD3_ASCII_RULES),
         "Validate strings according to STD3 rules"},
};

static const struct info ai_family_info[] =
{
        {AF_UNIX, STR(AF_UNIX), " Local communication"},
        {AF_LOCAL, STR(AF_LOCAL), " Local communication"},
        {AF_INET, STR(AF_INET), "IPv4 Internet protocols"},
        {AF_INET6, STR(AF_INET6), "IPv6 Internet protocols"},
        {AF_IPX, STR(AF_IPX), "IPX - Novell protocols"},
        {AF_NETLINK, STR(AF_NETLINK), "Kernel user interface device"},
        {AF_X25, STR(AF_X25), "ITU-T X.25 / ISO-8208 protocol"},
        {AF_AX25, STR(AF_AX25), "Amateur radio AX.25 protocol"},
        {AF_ATMPVC, STR(AF_ATMPVC), "Access to raw ATM PVCs"},
        {AF_APPLETALK, STR(AF_APPLETALK), "Appletalk"},
        {AF_PACKET, STR(AF_PACKET), "Low level packet interface"},
};

static const struct info ai_socktype_info[] =
{
};

static void
present_failed_status(int status)
{
        switch (status)
        {
        case EAI_ADDRFAMILY:
                printf("Failed with status: EAI_ADDRFAMILY.\n");
                break;
        case EAI_AGAIN:
                printf("Failed with status: EAI_AGAIN.\n");
                break;
        case EAI_BADFLAGS:
                printf("Failed with status: EAI_BADFLAGS.\n");
                break;
        case EAI_FAIL:
                printf("Failed with status: EAI_FAIL.\n");
                break;
        case EAI_FAMILY:
                printf("Failed with status: EAI_FAMILY.\n");
                break;
        case EAI_MEMORY:
                printf("Failed with status: EAI_MEMORY.\n");
                break;
        case EAI_NODATA:
                printf("Failed with status: EAI_NODATA.\n");
                break;
        case EAI_NONAME:
                printf("Failed with status: EAI_NONAME.\n");
                break;
        case EAI_SERVICE:
                printf("Failed with status: EAI_SERVICE.\n");
                break;
        case EAI_SOCKTYPE:
                printf("Failed with status: EAI_SOCKTYPE.\n");
                break;
        case EAI_SYSTEM:
                printf("Failed with status: EAI_SYSTEM.\n");
                break;
        default:
                printf("Failed with unknown status.\n");
                break;
        }
}

static void
present_info(const char *msg, enum info_comparison compare, uint32_t flags,
             const struct info *flags_and_desc,
             size_t length)
{
        unsigned int i;

        for (i = 0; i < length; i++)
        {
                const struct info *curr = &flags_and_desc[i];
                int match;

                switch (compare)
                {
                case INFO_COMPARISON_FLAG:
                        match = (flags & curr->flag);
                        break;
                case INFO_COMPARISON_VALUE:
                        match = (flags == curr->flag);
                        break;
                default:
                        match = 0;
                }
                
                if (match)
                {
                        printf("%s%s - %s.\n", msg, curr->name, curr->desc);
                }
        }
}

static void present_ai_flags(int order, int ai_flags)
{
        char msg1[100];
        char msg2[100];

        sprintf(msg1, "[%d]->ai_flags = ", order);
        sprintf(msg2, "%-20s", msg1);
        present_info(msg2, INFO_COMPARISON_FLAG, ai_flags, ai_flags_info,
                     ARRAY_SIZE(ai_flags_info));
}

static void present_ai_family(int order, int ai_family)
{
        char msg1[100];
        char msg2[100];

        sprintf(msg1, "[%d]->ai_family = ", order);
        sprintf(msg2, "%-20s", msg1);
        present_info(msg2, INFO_COMPARISON_VALUE, ai_family, ai_family_info,
                     ARRAY_SIZE(ai_family_info));
}

static void present_ai_socktype(int order, int ai_socktype)
{
}

static void present_ai_protocol(int order, int ai_protocol)
{
}

static void present_ai_addrlen(int order, size_t ai_addrlen)
{
}

static void present_ai_canonname(int order, char *ai_canonname)
{
}

static void present_addr_info(const struct addrinfo *addrinfo)
{
        static int order = 0;
        
        present_ai_family(order, addrinfo->ai_family);
        present_ai_flags(order, addrinfo->ai_flags);
        present_ai_socktype(order, addrinfo->ai_socktype);
        present_ai_protocol(order, addrinfo->ai_protocol);
        present_ai_addrlen(order, addrinfo->ai_addrlen);
        present_ai_canonname(order, addrinfo->ai_canonname);

        if (addrinfo->ai_next != NULL)
        {
                printf("\n");
                order++;
                present_addr_info(addrinfo->ai_next);
        }
}

void print_addr_info(const char *address)
{
        struct addrinfo *res;
        int status;

        printf("Printing addrinfo for address: \"%s\".\n", address);
        status = getaddrinfo(address, NULL, NULL, &res);
        if (status != 0)
        {
                present_failed_status(status);
                return;
        }

        present_addr_info(res);
        
        freeaddrinfo(res);
}

