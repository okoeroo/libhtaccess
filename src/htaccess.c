#include "htaccess/htaccess.h"
#include "htaccess_internal.h"


int
htaccess_parse_and_load(const char *buf) {
    int lineno = 0;
    int indent = 0;
    htaccess_ctx_t *ht_ctx;

    ht_ctx = new_htaccess_ctx();
    if (!ht_ctx)
        return 1;

    return htaccess_parse_directory(buf, &lineno, &indent, ht_ctx);
}

char *
htaccess_readfile(const char *fname) {
    FILE *fp;
    size_t size;
    char *buf = NULL;

    fp = fopen(fname, "r");
    if (!fp)
        goto final;

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    buf = malloc(size);
    if (!buf)
        goto final;

    if (fread(buf, 1, size, fp) != size) {
        goto final;
    }
    buf[size] = '\0';

    fclose(fp);
    return buf;

final:
    free(buf);
    return NULL;
}

