#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
        int opt;
        int flag_description = 0;

        opt = getopt(argc, argv, "d");
        if (opt == 'd')
        {
                argc--;
                argv++;
                flag_description = 1;
        }
        else if (opt != -1)
        {
                printf("Unknown option: -%c.\n\n", opt);
                return 1;
        }

        if (argc == 1)
        {
                printf("Must specify a valid address, for example "
                       "www.google.com.\n\n");
                return 2;
        }
        
        print_addr_info(argv[1]);
        
        return 0;
}
