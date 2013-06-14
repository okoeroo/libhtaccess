#include "htaccess/htaccess.h"
#include "htaccess_internal.h"


int
htaccess_parse_and_load(const char *fname) {
    int rc;
    char *buf;
    htaccess_ctx_t *ht_ctx;

    buf = htaccess_readfile(fname);
    if (!buf) {
        printf("Reading the htaccess file \"%s\" into a buffer failed.\n",
                fname);
        return 1;
    }

    ht_ctx = new_htaccess_ctx();
    if (!ht_ctx)
        return 1;

    rc = htaccess_parse_directory(buf, ht_ctx);
    if (rc < 0) {
        printf("Parsing failed\n");
        return 1;
    }

    htaccess_process_ctx(ht_ctx);
    return 0;
}

