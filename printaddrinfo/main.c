#include <stdio.h>
#include <unistd.h>

static void print_syntax(void)
{
        printf("SYNTAX: printaddrinfo [-d] [HOST[:PORT]]\n\n");
}

int main(int argc, char **argv)
{
        int opt;
        int flag_description = 0;
        const char *address;

        opt = getopt(argc, argv, "d");
        if (opt == 'd')
        {
                argc--;
                argv++;
                flag_description = 1;
        }
        else if (opt != -1)
        {
                printf("\nUNKNOWN OPTION: -%c.\n\n", opt);
                print_syntax();
                return 1;
        }

        address = (argc == 1) ? NULL : argv[1];
        print_addr_info(address, flag_description);
        printf("\n");
        
        return 0;
}
