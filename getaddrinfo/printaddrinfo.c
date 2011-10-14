#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>

#include "printaddrinfo.h"

static void
present_failed_status(int status)
{
        switch (status)
        {
        case EAI_ADDRFAMILY:
                printf("clli: Failed with status: EAI_ADDRFAMILY.\n");
                break;
        case EAI_AGAIN:
                printf("clli: Failed with status: EAI_AGAIN.\n");
                break;
        case EAI_BADFLAGS:
                printf("clli: Failed with status: EAI_BADFLAGS.\n");
                break;
        case EAI_FAIL:
                printf("clli: Failed with status: EAI_FAIL.\n");
                break;
        case EAI_FAMILY:
                printf("clli: Failed with status: EAI_FAMILY.\n");
                break;
        case EAI_MEMORY:
                printf("clli: Failed with status: EAI_MEMORY.\n");
                break;
        case EAI_NODATA:
                printf("clli: Failed with status: EAI_NODATA.\n");
                break;
        case EAI_NONAME:
                printf("clli: Failed with status: EAI_NONAME.\n");
                break;
        case EAI_SERVICE:
                printf("clli: Failed with status: EAI_SERVICE.\n");
                break;
        case EAI_SOCKTYPE:
                printf("clli: Failed with status: EAI_SOCKTYPE.\n");
                break;
        case EAI_SYSTEM:
                printf("clli: Failed with status: EAI_SYSTEM.\n");
                break;
        default:
                printf("clli: Failed with unknown status.\n");
                break;
        }
}

static void present_ai_flags(int ai_flags)
{
        printf("clli: addrinfo->ai_flags = ");
        switch (ai_flags)
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

static void present_ai_family(int ai_family)
{
}

static void present_ai_socktype(int ai_socktype)
{
}

static void present_ai_protocol(int ai_protocol)
{
}

static void present_ai_addrlen(size_t ai_addrlen)
{
}

static void present_ai_canonname(char *ai_canonname)
{
}

static void present_addr_info(const struct addrinfo *addrinfo)
{
        present_ai_flags(addrinfo->ai_flags);
        present_ai_family(addrinfo->ai_family);
        present_ai_socktype(addrinfo->ai_socktype);
        present_ai_protocol(addrinfo->ai_protocol);
        present_ai_addrlen(addrinfo->ai_addrlen);
        present_ai_canonname(addrinfo->ai_canonname);

        if (addrinfo->ai_next != NULL)
        {
                present_addr_info(addrinfo);
        }
}

void print_addr_info(const char *address)
{
        struct addrinfo *res;
        int status;

        status = getaddrinfo(address, NULL, NULL, &res);
        if (status != 0)
        {
                present_failed_status(status);
                return;
        }

        present_addr_info(res);
        
        freeaddrinfo(res);
}

