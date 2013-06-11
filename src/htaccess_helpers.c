#include "htaccess/htaccess.h"
#include "htaccess_internal.h"


/***** RB stuff *****/

RB_GENERATE(directive_tree_t, rb_directive_s, entry, htaccess_directive_cmp)

#define xdirective_add(xdirective, cstr) do {                     \
        htaccess_directive_t *c = malloc(sizeof(htaccess_directive_t));  \
                                                                  \
        c->type = xdirective;                                     \
        c->str  = cstr;                                           \
                                                                  \
        RB_INSERT(directive_tree_t, &directive_head, c);            \
} while (0)


struct directive_tree_t directive_head;
int xdirective_tree_initialized = 0;

int
htaccess_directive_cmp(htaccess_directive_t *a, htaccess_directive_t *b) {
    return b->type - a->type;
}

void
directive_list_init(void) {
    if (xdirective_tree_initialized) {
        /* Already initialized. */
        return;
    }

    RB_INIT(&directive_head);

    /* Initializations */
    xdirective_add(AUTHNAME, "http://www.w3.org/2001/XMLSchema#string");

    xdirective_tree_initialized = 1;
    return;
}

const char *
directive_to_str(htaccess_directive_type_t type) {
    htaccess_directive_t c;
    htaccess_directive_t *found;

    c.type = type;

    if (!(found = RB_FIND(directive_tree_t, &directive_head, &c))) {
        return "unknown";
    }

    return found->str;
}

RB_GENERATE(rb_directive_kv_list_head_t, rb_directive_kv_s, next, htaccess_directive_kv_cmp)
RB_GENERATE(rb_file_list_head_t, rb_file_s, next, htaccess_file_cmp)
RB_GENERATE(rb_directory_list_head_t, rb_directory_s, next, htaccess_directory_cmp)

int htaccess_directive_kv_cmp(struct rb_directive_kv_s *a,
                              struct rb_directive_kv_s *b) {
    int rc;
    if (!a && !b)
        return 0;

    /* printf("htaccess_directive_kv_cmp(%s, %s)\n", a->directive_kvname, b->directive_kvname); */
    rc = a->key - b->key;
    if (rc == 0)
        return strcmp(a->value, b->value);

     return rc;
}

int htaccess_file_cmp(struct rb_file_s *a,
                      struct rb_file_s *b) {
    if ((!a && !b) || (!a->filename && !b->filename))
        return 0;

    /* printf("htaccess_file_cmp(%s, %s)\n", a->filename, b->filename); */
    return strcmp(a->filename, b->filename);
}

int
htaccess_directory_cmp(htaccess_directory_t *a,
                       htaccess_directory_t *b) {
    if ((!a && !b) || (!a->dirname && !b->dirname))
        return 0;

    /* printf("htaccess_directory_cmp(%s, %s)\n", a->dirname, b->dirname); */
    return strcmp(a->dirname, b->dirname);
}



/* new_htaccess_directive_kv() expects a key to be set.
   v_loc: set to 0, and new_htaccess_directive_kv() will NOT allocate memory.
          The expectation is that the caller has already allocated the memory
          for the key.
          set to 1, and new_htaccess_directive_kv() will allocate memory for
          the null-terminated string.
 */
htaccess_directive_kv_t *
new_htaccess_directive_kv(const char *key, char *value, short v_loc) {
    htaccess_directive_kv_t *hta_dir_kv;

    if (!key)
        return NULL;

    hta_dir_kv = malloc(sizeof(htaccess_directive_kv_t));
    if (!hta_dir_kv)
        return NULL;

    memset(hta_dir_kv, 0, sizeof(htaccess_directive_kv_t));

    if (v_loc && value)
        hta_dir_kv->value = strdup(value);

    return hta_dir_kv;
error:
    free(hta_dir_kv);
    return NULL;
}

htaccess_file_t *
new_htaccess_file(void) {
    htaccess_file_t *file;

    file = malloc(sizeof(htaccess_file_t));
    if (!file)
        goto error;

    file->filename = NULL;
    return file;

error:
    free(file);
    return NULL;
}

htaccess_directory_t *
new_htaccess_directory(void) {
    htaccess_directory_t *dir;

    dir = malloc(sizeof(htaccess_directory_t));
    if (!dir)
        goto error;

    RB_INIT(&(dir->files));
    dir->dirname = NULL;
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
free_htaccess_directive_kv(htaccess_directive_kv_t *kv) {
    if (!kv)
        return;

    free(kv->key);
    free(kv->value);
    free(kv);
    return;
}

void
free_htaccess_file(htaccess_file_t *fn) {
    if (!fn)
        return;

    free(fn->filename);
    free(fn);
    return;
}

void
free_htaccess_directory(htaccess_directory_t *dir) {
    htaccess_file_t *fn, *tmp_fn;
    if (!dir)
        return;

    RB_FOREACH_SAFE(fn, rb_file_list_head_t, &(dir->files), tmp_fn) {
        free_htaccess_file(fn);
    }

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

