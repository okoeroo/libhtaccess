#include <stdio.h>
#include <htaccess/htaccess.h>


int
main (int argc, char *argv[]) {
    const char *fname = NULL;

    if (argc != 2) {
        printf("unknown amount of arguments, nothing to test with\n");
        return 1;
    }
    fname = argv[1];

    return htaccess_parse_and_load(fname);
}

