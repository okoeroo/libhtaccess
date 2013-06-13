#include "htaccess/htaccess.h"
#include "htaccess_internal.h"


int
htaccess_parse_and_load(const char *buf) {
    int rc;
    htaccess_ctx_t *ht_ctx;
    htaccess_directive_kv_t *hta_dir_kv_found, hta_dir_kv_search;

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
