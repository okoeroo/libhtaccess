#include "htaccess/htaccess.h"
#include "htaccess_internal.h"

int
htaccess_parse_buffer(htaccess_ctx_t *ht_ctx, char *buf) {
    int rc;

    if (!ht_ctx || !buf)
        return 1;

    rc = htaccess_parse_directory(buf, ht_ctx);
    if (rc < 0) {
        printf("Parsing failed\n");
        return 1;
    }

    htaccess_process_ctx(ht_ctx);
    return 0;
}

int
htaccess_parse_file(htaccess_ctx_t *ht_ctx, const char *fname) {
    int rc;
    char *buf;

    if (!ht_ctx || !fname)
        return 1;

    buf = htaccess_readfile(fname);
    if (!buf) {
        printf("Reading the htaccess file \"%s\" into a buffer failed.\n",
                fname);
        return 1;
    }

    return htaccess_parse_buffer(ht_ctx, buf);
}
