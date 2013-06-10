#include <stdio.h>
#include <htaccess/htaccess.h>


int
main (int argc, char *argv[]) {
    char *buf = NULL;
    const char *fname = NULL;

    if (argc != 2) {
        printf("unknown amount of arguments, nothing to test with\n");
        return 1;
    }
    fname = argv[1];

    buf = htaccess_readfile(fname);
    if (!buf) {
        printf("Reading the htaccess file \"%s\" into a buffer failed.\n",
                argv[1]);
        return 1;
    }
    /* printf("\n%s\n", buf); */

    return htaccess_parse_and_load(buf);
}

