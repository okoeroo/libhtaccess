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
    htaccess_print_ctx(ht_ctx);


    htaccess_directory_t *hta_dir;
    htaccess_file_t *hta_file;
    htaccess_directive_value_t *hta_dir_value;

    printf("----------\n");

    RB_FOREACH(hta_dir, rb_directory_list_head_t, &(ht_ctx->directories)) {
        printf("hta_dir->dirname: %s\n", hta_dir->dirname);
        RB_FOREACH(hta_file, rb_file_list_head_t, &(hta_dir->files)) {
            printf("\thta_file->filename: %s\n", hta_file->filename);

            hta_dir_kv_search.key->type = AUTHGROUPFILE;
            /* hta_dir_kv_search.type = AUTHUSERFILE; */

            hta_dir_kv_found = RB_FIND(rb_directive_kv_list_head_t, &(hta_file->directives), &hta_dir_kv_search);
            if (hta_dir_kv_found) {
                TAILQ_FOREACH(hta_dir_value, &(hta_dir_kv_found->values), next) {
                    printf("\t\t\thta_dir_value->value: %s\n", hta_dir_value->value);
                }
            }

        }
    }

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
