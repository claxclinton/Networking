#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>

#include "printaddrinfo.h"

#define STR(x) #x

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*(array)))

struct info_elem;

static int check_flags(uint32_t elem_value, uint32_t ai_value);
static int compare_ls_nibble(uint32_t elem_value, uint32_t ai_value);

struct info_elem
{
        const uint32_t value;
        const char *name;
        const char *desc;
};

struct info
{
        int (*compare)(uint32_t elem_value, uint32_t ai_value);
        const size_t num_elems;
        const struct info_elem *info_elems;
};

static const struct info_elem ai_flags_info_elems[] =
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

static const struct info ai_flags_info =
{
        check_flags,
        ARRAY_SIZE(ai_flags_info_elems),
        ai_flags_info_elems
};

static const struct info_elem ai_family_info_elems[] =
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

static const struct info ai_family_info =
{
        compare_ls_nibble,
        ARRAY_SIZE(ai_family_info_elems),
        ai_family_info_elems
};

static int check_flags(uint32_t elem_value, uint32_t ai_value)
{
        return elem_value & ai_value;
}

static int compare_ls_nibble(uint32_t elem_value, uint32_t ai_value)
{
        return (ai_value & 0x0000000FUL) == elem_value;
}

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
present_info(const char *msg, uint32_t ai_value, const struct info *info)
{
        unsigned int i;

        for (i = 0; i < info->num_elems; i++)
        {
                const struct info_elem *elem = &info->info_elems[i];
                
                if (info->compare(elem->value, ai_value))
                {
                        printf("%s%s - %s.\n", msg, elem->name, elem->desc);
                }
        }
}

static void present_ai_flags(int order, int ai_flags)
{
        char msg1[100];
        char msg2[100];

        sprintf(msg1, "[%d]->ai_flags = ", order);
        sprintf(msg2, "%-20s", msg1);
        present_info(msg2, ai_flags, &ai_flags_info);
}

static void present_ai_family(int order, int ai_family)
{
        char msg1[100];
        char msg2[100];

        sprintf(msg1, "[%d]->ai_family = ", order);
        sprintf(msg2, "%-20s", msg1);
        present_info(msg2, ai_family, &ai_family_info);
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

