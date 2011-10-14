/* This example will request address info for www.google.com,
 */

#include <stdio.h>

int main(int argc, const char **argv)
{
        printf("clli: Testing out getaddrinfo.\n");
        print_addr_info("www.google.com");
        
        return 0;
}
