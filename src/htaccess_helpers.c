#include "htaccess/htaccess.h"
#include "htaccess_internal.h"


/***** RB stuff *****/

#if 0

const char *
datatype_to_str(ga_xacml_datatype_t type) {
    struct datatype   c;
    struct datatype * found;

    c.type = type;

    if (!(found = RB_FIND(datatype_tree, &datatype_head, &c))) {
        return "unknown";
    }

    return found->str;
}
#endif


int htaccess_files_cmp(struct rb_files_s *a,
                       struct rb_files_s *b) {
    return strcmp(a->filename, b->filename);
}
RB_GENERATE(rb_files_list_head_t, rb_files_s, next, htaccess_files_cmp)

int
htaccess_directory_cmp(htaccess_directory_t *a,
                       htaccess_directory_t *b) {
    return strcmp(a->dirname, b->dirname);
}
RB_GENERATE(rb_directory_list_head_t, rb_directory_s, next, htaccess_directory_cmp)




htaccess_directory_t *
new_htaccess_directory(void) {
    htaccess_directory_t *dir;

    dir = malloc(sizeof(htaccess_directory_t));
    if (!dir)
        goto error;

    memset(dir, 0, sizeof(htaccess_directory_t));
    return dir;

error:
    free(dir);
    return NULL;
}

htaccess_ctx_t *
new_htaccess_ctx(void) {
    htaccess_ctx_t *ctx = NULL;

    ctx = malloc(sizeof(htaccess_ctx_t));
    if (!ctx)
        goto error;

    RB_INIT(&(ctx->directories));

    return ctx;

error:
    free(ctx);
    return NULL;
}


void
free_htaccess_directory(htaccess_directory_t *dir) {
    if (!dir)
        return;

    free(dir->dirname);
    free(dir);
    return;
}

void
free_htaccess_ctx(htaccess_ctx_t *ctx) {
    htaccess_directory_t *dir, *tmp_dir;

    if (!ctx)
        return;

    RB_FOREACH_SAFE(dir, rb_directory_list_head_t, &(ctx->directories), tmp_dir) {
        printf("dir->dirname: %s\n", dir->dirname);
        free_htaccess_directory(dir);
    }

    free(ctx);
    return;
}


/***** RB stuff *****/


char *
htaccess_parse_quoted_string(const char *buf) {
    unsigned int i;
    char *str;
    const char *buf2;

    if (buf[0] != '\"')
        return NULL;

    buf2 = &buf[1];

    /* Seek the end */
    for (i = 0; i < strlen(buf2); i++) {
        if (buf2[i] == '\"') {
            break;
        }
    }

    str = calloc(i + 1, 1);
    if (!str)
        return NULL;

    memcpy(str, buf2, i);
    str[i] = '\0';

    return str;
}

int
htaccess_count_token(const char *buf, const char *tokens) {
    int i, j, bsz, tsz, cnt = 0;

    bsz = strlen(buf);
    tsz = strlen(tokens);
    for (i = 0; i < bsz; i++) {
        for (j = 0; j < tsz; j++) {
            if (buf[i] == buf[j]) {
                cnt++;
            } else {
                return cnt;
            }
        }
    }
    return cnt;
}

char *
htaccess_str_returned_upto_EOL(const char *buf) {
    int i = 0;
    char *str;

    while (1) {
        if (buf[i] == '\0' || buf[i] == '\n')
            break;
        else
            i++;
    }
    str = malloc(i + 1);
    if (!str)
        return NULL;

    memcpy(str, buf, i);
    str[i] = '\0';

    return str;
}

