#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>

#include "printaddrinfo.h"

#define STR(x) #x
#if 0
#define STRSTR(x) STR(x)
#endif

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*(array)))

struct flags_and_desc
{
        uint32_t flag;
        const char *name;
        const char *desc;
};

static const struct flags_and_desc ai_flags_and_desc[] =
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
present_flags_and_desc(const char *msg, uint32_t flags,
                       const struct flags_and_desc *flags_and_desc,
                       size_t length)
{
        unsigned int i;

        for (i = 0; i < length; i++)
        {
                const struct flags_and_desc *curr = &flags_and_desc[i];

                if (flags & curr->flag)
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
        present_flags_and_desc(msg2, ai_flags, ai_flags_and_desc,
                               ARRAY_SIZE(ai_flags_and_desc));
}

static void present_ai_family(int order, int ai_family)
{
        char msg1[100];
        char msg2[100];

        sprintf(msg1, "[%d]->ai_family = ", order);
        sprintf(msg2, "%-20s", msg1);
        printf("%s", msg2);
        
        switch (ai_family)
        {
        case AF_INET:
                printf("AF_INET");
                break;
        case AF_INET6:
                printf("AF_INET6");
                break;
        case AF_UNSPEC:
                printf("AF_UNSPEC");
                break;
        default:
                printf("UNKNOWN");
                break;
        }
        printf("\n");
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
        
        present_ai_flags(order, addrinfo->ai_flags);
        present_ai_family(order, addrinfo->ai_family);
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

