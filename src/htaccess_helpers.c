#include "htaccess/htaccess.h"
#include "htaccess_internal.h"


/***** RB stuff *****/

RB_GENERATE(directive_map_tree_t, rb_directive_map_s, entry, htaccess_directive_map_cmp)

#define xdirective_map_add(xdirective, cstr) do {                     \
        htaccess_directive_map_t *c = malloc(sizeof(htaccess_directive_map_t));  \
                                                                  \
        c->type = xdirective;                                     \
        c->str  = cstr;                                           \
                                                                  \
        RB_INSERT(directive_map_tree_t, &directive_map_head, c);            \
} while (0)


struct directive_map_tree_t directive_map_head;
int xdirective_map_tree_initialized = 0;

int
htaccess_directive_map_cmp(htaccess_directive_map_t *a, htaccess_directive_map_t *b) {
    return strcasecmp(a->str, b->str);
    /* return b->type - a->type; */
}

void
directive_map_list_init(void) {
    if (xdirective_map_tree_initialized) {
        /* Already initialized. */
        return;
    }

    RB_INIT(&directive_map_head);

    /* Initializations */
    xdirective_map_add(AUTHNAME, "AuthName");
    xdirective_map_add(AUTHGROUPFILE, "AuthGroupFile");
    xdirective_map_add(REQUIRE_GROUP, "Require Group");
    xdirective_map_add(ORDER, "Order");
    xdirective_map_add(DENY_FROM, "Deny From");
    xdirective_map_add(ALLOW_FROM, "Allow From");
    xdirective_map_add(AUTHTYPE, "AuthType");
    xdirective_map_add(AUTHUSERFILE, "AuthUserFile");
    xdirective_map_add(REQUIRE, "Require");
    xdirective_map_add(REDIRECT, "Redirect");
    xdirective_map_add(SETENVIF, "SetEnvIf");
    xdirective_map_add(REWRITECOND, "RewriteCond");
    xdirective_map_add(REWRITERULE, "RewriteRule");
    xdirective_map_add(DIRECTORYINDEX, "DirectoryIndex");
    xdirective_map_add(ADDHANDLER, "AddHandler");
    xdirective_map_add(ERRORDOCUMENT, "ErrorDocument");
    xdirective_map_add(ADDDEFAULTCHARSET, "AddDefaultCharset");
    xdirective_map_add(CHARSETSOURCEENC, "CharsetSourceEnc");

    xdirective_map_tree_initialized = 1;
    return;
}

htaccess_directive_map_t *
search_directive_map(const char *s) {
    htaccess_directive_map_t *dir_map_found, dir_map_search;

    dir_map_search.str = s;
    dir_map_found = RB_FIND(directive_map_tree_t, &directive_map_head, &dir_map_search);

    return dir_map_found;
}

const char *
directive_map_to_str(htaccess_directive_type_t type) {
    htaccess_directive_map_t c;
    htaccess_directive_map_t *found;

    c.type = type;

    if (!(found = RB_FIND(directive_map_tree_t, &directive_map_head, &c))) {
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

    if (v_loc && value) {
        hta_dir_kv->value = strdup(value);
        if (!hta_dir_kv->value)
            goto error;
    }

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

    directive_map_list_init();

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

