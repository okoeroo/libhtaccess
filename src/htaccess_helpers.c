#include "htaccess_internal.h"


/***** RB stuff *****/
RB_GENERATE(rb_htpasswd_tree_t, rb_htpasswd_s, entry, htaccess_htpasswd_cmp)
RB_GENERATE(rb_htgroup_tree_t, rb_htgroup_s, entry, htaccess_htgroup_cmp)

int htaccess_htpasswd_cmp(htaccess_htpasswd_t *a, htaccess_htpasswd_t *b) {
    if ((!a && !b) || (!a->username && !b->username))
        return 0;

    return strcmp(a->username, b->username);
}

int htaccess_htgroup_cmp(htaccess_htgroup_t *a, htaccess_htgroup_t *b) {
    int rc;
    if (!a && !b)
        return 0;

    if (!a->groupname && !b->groupname)
        return 0;

    rc = strcmp(a->groupname, b->groupname);
    if (rc != 0)
        return rc;

    if (!a->username && !b->username)
        return 0;

    return strcmp(a->username, b->username);
}

RB_GENERATE(rb_filepath_tree_t, rb_filepath_s, entry, htaccess_filepath_cmp)

int htaccess_filepath_cmp(htaccess_filepath_t *a, htaccess_filepath_t *b) {
    if ((!a && !b) || (!a->path && !b->path))
        return 0;

    return strcmp(a->path, b->path);
}


htaccess_htpasswd_t *new_htaccess_htpasswd(void) {
    htaccess_htpasswd_t *pw;

    pw = calloc(1, sizeof(htaccess_htpasswd_t));
    if (!pw)
        return NULL;

    return pw;
}

void free_htaccess_htpasswd(htaccess_htpasswd_t *pw) {
    if (!pw)
        return;

    free(pw->username);
    free(pw->pwhash);
    free(pw);
}

htaccess_htgroup_t *new_htaccess_htgroup(void) {
    htaccess_htgroup_t *gr;

    gr = calloc(1, sizeof(htaccess_htgroup_t));
    if (!gr)
        return NULL;

    return gr;
}

void free_htaccess_htgroup(htaccess_htgroup_t *gr) {
    if (!gr)
        return;

    free(gr->groupname);
    free(gr->username);
    free(gr);
}

htaccess_filepath_t *
new_htaccess_filepath(void) {
    htaccess_filepath_t *filepath;

    filepath = calloc(1, sizeof(htaccess_filepath_t));
    if (!filepath)
        return NULL;

    RB_INIT(&(filepath->htpasswd));
    RB_INIT(&(filepath->htgroup));

    return filepath;
}

void
free_htaccess_filepath(htaccess_filepath_t *filepath) {
    htaccess_htpasswd_t *pw, *pw_tmp;
    htaccess_htgroup_t *gr, *gr_tmp;

    RB_FOREACH_SAFE(pw, rb_htpasswd_tree_t, &(filepath->htpasswd), pw_tmp) {
        RB_REMOVE(rb_htpasswd_tree_t, &(filepath->htpasswd), pw);
        free_htaccess_htpasswd(pw);
    }

    RB_FOREACH_SAFE(gr, rb_htgroup_tree_t, &(filepath->htgroup), gr_tmp) {
        RB_REMOVE(rb_htgroup_tree_t, &(filepath->htgroup), gr);
        free_htaccess_htgroup(gr);
    }

    free(filepath);
    return;
}

htaccess_filepath_t *
htaccess_search_filepath(htaccess_ctx_t *ht_ctx, char *path) {
    htaccess_filepath_t search;

    search.path = path;
    return RB_FIND(rb_filepath_tree_t, &(ht_ctx->paths), &search);
}

htaccess_filepath_t *
htaccess_add_filepath(htaccess_ctx_t *ht_ctx, char *path) {
    htaccess_filepath_t *hta_filepath;

    hta_filepath = htaccess_search_filepath(ht_ctx, path);
    if (hta_filepath)
        return hta_filepath;

    hta_filepath = malloc(sizeof(htaccess_filepath_t));
    if (!hta_filepath)
        return NULL;

    hta_filepath->path = path;
    RB_INSERT(rb_filepath_tree_t, &(ht_ctx->paths), hta_filepath);
    return hta_filepath;
}


RB_GENERATE(directive_map_tree_t, rb_directive_map_s, entry, htaccess_directive_map_cmp)

#define xdirective_map_add(xdirective, cstr, pcnt, q) do {        \
        htaccess_directive_map_t *c = malloc(sizeof(htaccess_directive_map_t));  \
                                                                  \
        c->type = xdirective;                                     \
        c->str  = cstr;                                           \
        c->len  = strlen(cstr);                                   \
        c->par_cnt = pcnt;                                        \
        c->quotation = q;                                         \
                                                                  \
        RB_INSERT(directive_map_tree_t, &directive_map_head, c);  \
} while (0)

RB_GENERATE(sub_directive_map_tree_t, rb_sub_directive_map_s, entry, htaccess_sub_directive_map_cmp)

#define xsub_directive_map_add(sub_xdirective, cstr) do {        \
        htaccess_sub_directive_map_t *c = malloc(sizeof(htaccess_sub_directive_map_t));  \
                                                                  \
        c->type = sub_xdirective;                                 \
        c->str  = cstr;                                           \
                                                                  \
        RB_INSERT(sub_directive_map_tree_t, &sub_directive_map_head, c);  \
} while (0)



struct directive_map_tree_t directive_map_head;
struct sub_directive_map_tree_t sub_directive_map_head;
unsigned short xdirective_map_tree_initialized = 0;
unsigned short xsub_directive_map_tree_initialized = 0;

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
    xdirective_map_add(HTA_AUTHNAME, "AuthName", 1, HTA_MUST_QUOTE);
    xdirective_map_add(HTA_AUTHGROUPFILE, "AuthGroupFile", 1, HTA_OPT_QUOTE);
    /* xdirective_map_add(HTA_REQUIRE_GROUP, "Require Group", 1, HTA_NO_QUOTE); */
    xdirective_map_add(HTA_ORDER, "Order", 2, HTA_NO_QUOTE);
    xdirective_map_add(HTA_DENY_FROM, "Deny From", 1, HTA_NO_QUOTE);
    xdirective_map_add(HTA_ALLOW_FROM, "Allow From", 1, HTA_NO_QUOTE);
    xdirective_map_add(HTA_AUTHTYPE, "AuthType", 1, HTA_NO_QUOTE);
    xdirective_map_add(HTA_AUTHUSERFILE, "AuthUserFile", 1, HTA_OPT_QUOTE);
    xdirective_map_add(HTA_REQUIRE, "Require", 99, HTA_NO_QUOTE);
    xdirective_map_add(HTA_REDIRECT, "Redirect", 2, HTA_NO_QUOTE);
    xdirective_map_add(HTA_SETENVIF, "SetEnvIf", 3, HTA_NO_QUOTE);
    xdirective_map_add(HTA_REWRITECOND, "RewriteCond", 2, HTA_NO_QUOTE);
    xdirective_map_add(HTA_REWRITERULE, "RewriteRule", 2, HTA_NO_QUOTE);
    xdirective_map_add(HTA_DIRECTORYINDEX, "DirectoryIndex", 1, HTA_NO_QUOTE);
    /* xdirective_map_add(HTA_ADDHANDLER, "AddHandler", 1-, HTA_NO_QUOTE); */
    xdirective_map_add(HTA_ERRORDOCUMENT, "ErrorDocument", 2, HTA_NO_QUOTE);
    xdirective_map_add(HTA_ADDDEFAULTCHARSET, "AddDefaultCharset", 1, HTA_NO_QUOTE);
    xdirective_map_add(HTA_CHARSETSOURCEENC, "CharsetSourceEnc", 1, HTA_NO_QUOTE);

    xdirective_map_tree_initialized = 1;
    return;
}

htaccess_directive_map_t *
search_directive_map_on_str(const char *s) {
    htaccess_directive_map_t *dir_map_found;

    if (!xdirective_map_tree_initialized)
        directive_map_list_init();

    RB_FOREACH(dir_map_found, directive_map_tree_t, &directive_map_head) {
        if (strncasecmp(s, dir_map_found->str, strlen(dir_map_found->str)) == 0) {
            return dir_map_found;
        }
    }
    return NULL;
}

htaccess_directive_map_t *
search_directive_map_on_type(htaccess_directive_type_t t) {
    htaccess_directive_map_t *dir_map_found;

    if (!xdirective_map_tree_initialized)
        directive_map_list_init();

    RB_FOREACH(dir_map_found, directive_map_tree_t, &directive_map_head) {
        if (dir_map_found->type == t) {
            return dir_map_found;
        }
    }
    return NULL;
}


int
htaccess_sub_directive_map_cmp(htaccess_sub_directive_map_t *a, htaccess_sub_directive_map_t *b) {
    return strcasecmp(a->str, b->str);
    /* return b->type - a->type; */
}

void
sub_directive_map_list_init(void) {
    if (xsub_directive_map_tree_initialized) {
        /* Already initialized. */
        return;
    }

    RB_INIT(&sub_directive_map_head);

    /* Initializations */
    xsub_directive_map_add(HTA_GROUP, "group");
    xsub_directive_map_add(HTA_ALL, "all");
    xsub_directive_map_add(HTA_ENV, "env");
    xsub_directive_map_add(HTA_METHOD, "method");
    xsub_directive_map_add(HTA_EXPR, "expr");
    xsub_directive_map_add(HTA_USER, "user");
    xsub_directive_map_add(HTA_VALID_USER, "valid-user");
    xsub_directive_map_add(HTA_IP, "ip");

    xsub_directive_map_tree_initialized = 1;
    return;
}

htaccess_sub_directive_map_t *
search_sub_directive_map_on_str(const char *s) {
    htaccess_sub_directive_map_t sub_dir_map_search;

    if (!xsub_directive_map_tree_initialized)
        sub_directive_map_list_init();

    sub_dir_map_search.str = s;
    return RB_FIND(sub_directive_map_tree_t, &sub_directive_map_head, &sub_dir_map_search);
}

htaccess_sub_directive_map_t *
search_sub_directive_map_on_type(htaccess_sub_directive_type_t t) {
    htaccess_sub_directive_map_t *sub_dir_map_found;

    if (!xsub_directive_map_tree_initialized)
        sub_directive_map_list_init();

    RB_FOREACH(sub_dir_map_found, sub_directive_map_tree_t, &sub_directive_map_head) {
        if (sub_dir_map_found->type == t) {
            return sub_dir_map_found;
        }
    }
    return NULL;
}


RB_GENERATE(rb_directive_kv_list_head_t, rb_directive_kv_s, next, htaccess_directive_kv_cmp)
RB_GENERATE(rb_file_list_head_t, rb_file_s, next, htaccess_file_cmp)
RB_GENERATE(rb_directory_list_head_t, rb_directory_s, next, htaccess_directory_cmp)

int htaccess_directive_kv_cmp(struct rb_directive_kv_s *a,
                              struct rb_directive_kv_s *b) {
    int rc;
    if ((!a && !b) || (!a->key && !b->key))
        return 0;

    /* printf("htaccess_directive_kv_cmp(%s, %s)\n", a->directive_kvname, b->directive_kvname); */
    rc = a->key->type - b->key->type;
    return rc;
}

int htaccess_file_cmp(struct rb_file_s *a,
                      struct rb_file_s *b) {
    if ((!a && !b) || (!a->filename && !b->filename))
        return 0;

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



/* new_htaccess_directive_value()
   v_loc: set to 0, and new_htaccess_directive_kv() will NOT allocate memory.
          The expectation is that the caller has already allocated the memory
          for the key.
          set to 1, and new_htaccess_directive_kv() will allocate memory for
          the null-terminated string.
 */
htaccess_directive_value_t *
new_htaccess_directive_value(char *value, unsigned short v_loc) {
    htaccess_directive_value_t *d_val;
    if (!value)
        return NULL;

    d_val = calloc(1, sizeof(htaccess_directive_value_t));
    if (!d_val)
        return NULL;

    d_val->v_loc = v_loc;
    if (d_val->v_loc) {
        d_val->value = strdup(value);
        if (!d_val->value)
            goto error;
    } else {
        d_val->value = value;
    }

    return d_val;
error:
    free(d_val);
    return NULL;
}

htaccess_directive_kv_t *
new_htaccess_directive_kv(htaccess_directive_map_t *key) {
    htaccess_directive_kv_t *hta_dir_kv;

    if (!key)
        return NULL;

    hta_dir_kv = calloc(1, sizeof(htaccess_directive_kv_t));
    if (!hta_dir_kv)
        return NULL;

    hta_dir_kv->key = key;
    TAILQ_INIT(&(hta_dir_kv->values));

    return hta_dir_kv;
}

htaccess_file_t *
new_htaccess_file(void) {
    htaccess_file_t *file;

    file = calloc(1, sizeof(htaccess_file_t));
    if (!file)
        goto error;

    return file;

error:
    free(file);
    return NULL;
}

htaccess_directory_t *
new_htaccess_directory(void) {
    htaccess_directory_t *dir;

    dir = calloc(1, sizeof(htaccess_directory_t));
    if (!dir)
        goto error;

    RB_INIT(&(dir->files));
    return dir;

error:
    free(dir);
    return NULL;
}

htaccess_ctx_t *
new_htaccess_ctx(void) {
    htaccess_ctx_t *ctx = NULL;

    ctx = calloc(1, sizeof(htaccess_ctx_t));
    if (!ctx)
        goto error;

    directive_map_list_init();
    sub_directive_map_list_init();

    RB_INIT(&(ctx->directories));

    /* Allocate error buffer */
    ctx->error = malloc(HTACCESS_MAX_ERROR_STR);
    if (!ctx->error)
        goto error;

    htaccess_clear_error(ctx);
    return ctx;
error:
    free(ctx->error);
    free(ctx);
    return NULL;
}

void
free_htaccess_directive_value(htaccess_directive_value_t *val) {
    if (!val)
        return;

    if (val->v_loc) {
        free(val->value);
    }
    val->value = NULL;

    free(val);
    return;
}

void
free_htaccess_directive_kv(htaccess_directive_kv_t *kv) {
    htaccess_directive_value_t *x_value, *x_value_tmp;
    if (!kv)
        return;

    TAILQ_FOREACH_SAFE(x_value, &(kv->values), next, x_value_tmp) {
        TAILQ_REMOVE(&(kv->values), x_value, next);
        free_htaccess_directive_value(x_value);
    }

    kv->key = NULL; /* Don't free here! */
    free(kv);
    return;
}

void
free_htaccess_file(htaccess_file_t *fn) {
    htaccess_directive_kv_t *kv, *tmp_kv;

    if (!fn)
        return;

    RB_FOREACH_SAFE(kv, rb_directive_kv_list_head_t, &(fn->directives), tmp_kv) {
        RB_REMOVE(rb_directive_kv_list_head_t, &(fn->directives), kv);
        free_htaccess_directive_kv(kv);
    }

    free(fn->filename);
    fn->filename = NULL;
    free(fn);
    return;
}

void
free_htaccess_directory(htaccess_directory_t *dir) {
    htaccess_file_t *fn, *tmp_fn;
    if (!dir)
        return;

    RB_FOREACH_SAFE(fn, rb_file_list_head_t, &(dir->files), tmp_fn) {
        RB_REMOVE(rb_file_list_head_t, &(dir->files), fn);
        free_htaccess_file(fn);
    }

    free(dir->dirname);
    dir->dirname = NULL;
    free(dir);
    return;
}

void
free_htaccess_ctx(htaccess_ctx_t *ctx) {
    htaccess_directory_t *dir, *tmp_dir;
    htaccess_filepath_t *fp, *tmp_fp;

    if (!ctx)
        return;

    RB_FOREACH_SAFE(dir, rb_directory_list_head_t, &(ctx->directories), tmp_dir) {
        RB_REMOVE(rb_directory_list_head_t, &(ctx->directories), dir);
        free_htaccess_directory(dir);
    }

    if (ctx->error)
        free(ctx->error);

    RB_FOREACH_SAFE(fp, rb_filepath_tree_t, &(ctx->paths), tmp_fp) {
        RB_REMOVE(rb_filepath_tree_t, &(ctx->paths), fp);
        free_htaccess_filepath(fp);
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
htaccess_copy_string(const char *buf) {
    int i = 0;
    char *str;

    if (!buf)
        return NULL;

    while (1) {
        if (isspace(buf[i]))
            break;
        i++;
    }

    str = malloc(i + 1);
    if (!str)
        return NULL;

    memcpy(str, buf, i);
    str[i] = '\0';

    return str;
}

char *
htaccess_str_returned_upto_colon(const char *buf) {
    int i = 0;
    char *str;

    while (1) {
        if (buf[i] == '\0' || buf[i] == ':')
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

int
htaccess_add_error(htaccess_ctx_t *ht_ctx, const char *fmt, ...) {
    va_list pvar;
    int res;
    char *buf = NULL;
    char *buf2 = NULL;

    if (!ht_ctx || !fmt)
        return 1;

    buf = malloc(HTACCESS_MAX_ERROR_STR);
    if (!buf)
        goto error;

    buf2 = malloc(HTACCESS_MAX_ERROR_STR);
    if (!buf2)
        goto error;

    va_start(pvar, fmt);
    res = vsnprintf(buf, HTACCESS_MAX_ERROR_STR, fmt, pvar);
    va_end(pvar);

    if ((res >= HTACCESS_MAX_ERROR_STR) || (res < 0))
        goto error;

    if (htaccess_get_error(ht_ctx)) {
        snprintf(buf2, HTACCESS_MAX_ERROR_STR, "%s: %s", buf, htaccess_get_error(ht_ctx));
        htaccess_set_error(ht_ctx, buf2);
    } else {
        htaccess_set_error(ht_ctx, buf);
    }

    free(buf);
    free(buf2);
    return 0;
error:
    free(buf);
    free(buf2);
    return 1;
}

int
htaccess_set_error(htaccess_ctx_t *ht_ctx, const char *fmt, ...) {
    va_list pvar;
    int res;

    if (!ht_ctx || !fmt)
        return 1;

    va_start(pvar, fmt);
    res = vsnprintf(ht_ctx->error, HTACCESS_MAX_ERROR_STR, fmt, pvar);
    va_end(pvar);

    if ((res >= HTACCESS_MAX_ERROR_STR) || (res < 0))
        return 1;

    return 0;
}

char *
htaccess_get_error(htaccess_ctx_t *ht_ctx) {
    if (!ht_ctx || !ht_ctx->error || ht_ctx->error[0] == '\0')
        return NULL;

    return ht_ctx->error;
}

void
htaccess_clear_error(htaccess_ctx_t *ctx) {
    if (!ctx || !ctx->error)
        return;

    memset(ctx->error, 0, HTACCESS_MAX_ERROR_STR);
    return;
}

void
htaccess_process_ctx(htaccess_ctx_t *ht_ctx) {
    htaccess_directory_t *hta_dir;
    htaccess_file_t *hta_file;
    htaccess_directive_value_t *hta_dir_value, *hta_dir_value_tmp;
    htaccess_directive_kv_t *hta_dir_kv, *hta_dir_kv_found, hta_dir_kv_search;
    htaccess_filepath_t *hta_filepath;

    size_t cnt;
    htaccess_sub_directive_map_t *sub_dir_match = NULL;

    RB_FOREACH(hta_dir, rb_directory_list_head_t, &(ht_ctx->directories)) {
        RB_FOREACH(hta_file, rb_file_list_head_t, &(hta_dir->files)) {

            hta_dir_kv_search.key = search_directive_map_on_type(HTA_AUTHGROUPFILE);
            /* hta_dir_kv_search.type = AUTHUSERFILE; */

            hta_dir_kv_found = RB_FIND(rb_directive_kv_list_head_t, &(hta_file->directives), &hta_dir_kv_search);
            if (hta_dir_kv_found) {
                TAILQ_FOREACH(hta_dir_value, &(hta_dir_kv_found->values), next) {
                    /* printf("\t\t\thta_dir_value->value: %s\n", hta_dir_value->value); */

                    /* Idea: 1. add it to the ctx.
                             2. add it to the hta_dir_value, parsed as AUTHGROUPFILE or AUTHUSERFILE object. */
                    hta_filepath = htaccess_add_filepath(ht_ctx, hta_dir_value->value);
                    hta_dir_value->filepath = hta_filepath;

                    htaccess_parse_htgroup(ht_ctx, hta_dir_value->filepath);
                }
            }

            hta_dir_kv_search.key = search_directive_map_on_type(HTA_AUTHUSERFILE);
            hta_dir_kv_found = RB_FIND(rb_directive_kv_list_head_t, &(hta_file->directives), &hta_dir_kv_search);
            if (hta_dir_kv_found) {
                TAILQ_FOREACH(hta_dir_value, &(hta_dir_kv_found->values), next) {
                    /* printf("\t\t\thta_dir_value->value: %s\n", hta_dir_value->value); */

                    /* Idea: 1. add it to the ctx.
                             2. add it to the hta_dir_value, parsed as AUTHGROUPFILE or AUTHUSERFILE object. */
                    hta_filepath = htaccess_add_filepath(ht_ctx, hta_dir_value->value);
                    hta_dir_value->filepath = hta_filepath;

                    htaccess_parse_htpasswd(ht_ctx, hta_dir_value->filepath);
                }
            }
            /* Remove values if they are sub-directive and label siblings as that sub-directive */
            RB_FOREACH(hta_dir_kv, rb_directive_kv_list_head_t, &(hta_file->directives)) {
                if (hta_dir_kv->key->type == HTA_REQUIRE) {
                    cnt = 0;
                    TAILQ_FOREACH_SAFE(hta_dir_value, &(hta_dir_kv->values), next, hta_dir_value_tmp) {
                        if (cnt == 0) {
                            /* Match sub directive */
                            sub_dir_match = search_sub_directive_map_on_str(hta_dir_value->value);
                            if (sub_dir_match) {
                                TAILQ_REMOVE(&(hta_dir_kv->values), hta_dir_value, next);
                                free_htaccess_directive_value(hta_dir_value);
                            }
                        } else {
                            /* Push the mapped sub directive match to all the values */
                            hta_dir_value->sub_directive = sub_dir_match;
                        }
                        cnt++;
                    }
                }
            }
        }
    }
#if 0
    htaccess_print_ctx(ht_ctx);
#endif

    return;
}

void
htaccess_print_ctx(htaccess_ctx_t *ht_ctx) {
    htaccess_directory_t *hta_dir;
    htaccess_file_t *hta_file;
    htaccess_directive_kv_t *hta_dir_kv;
    htaccess_directive_value_t *hta_dir_value;
    htaccess_htpasswd_t *pw;
    htaccess_htgroup_t *gr;
    htaccess_filepath_t *hta_filepath;


    RB_FOREACH(hta_dir, rb_directory_list_head_t, &(ht_ctx->directories)) {
        printf("hta_dir->dirname: %s\n", hta_dir->dirname);
        RB_FOREACH(hta_file, rb_file_list_head_t, &(hta_dir->files)) {
            printf("\thta_file->filename: %s\n", hta_file->filename);
            RB_FOREACH(hta_dir_kv, rb_directive_kv_list_head_t, &(hta_file->directives)) {
                printf("\t\thta_dir_kv->key->str: %s\n", hta_dir_kv->key->str);
                TAILQ_FOREACH(hta_dir_value, &(hta_dir_kv->values), next) {
                    printf("\t\t\thta_dir_value->value: %s\n", hta_dir_value->value);
                    if (hta_dir_value->sub_directive)
                        printf("\t\t\thta_dir_value->sub_directive->str: %s\n", hta_dir_value->sub_directive->str);

                    if (hta_dir_kv->key->type == HTA_AUTHUSERFILE) {
                        RB_FOREACH(pw, rb_htpasswd_tree_t, &(hta_dir_value->filepath->htpasswd)) {
                            printf("\t\t\t\tusername: \"%s\" password hash: \"%s\"\n",
                                    pw->username,
                                    pw->pwhash);
                        }
                    } else if (hta_dir_kv->key->type == HTA_AUTHGROUPFILE) {
                        RB_FOREACH(gr, rb_htgroup_tree_t, &(hta_dir_value->filepath->htgroup)) {
                            printf("\t\t\t\tgroupname: \"%s\" username: \"%s\"\n",
                                    gr->groupname,
                                    gr->username);
                        }
                    }
                }
            }
        }
    }
    RB_FOREACH(hta_filepath, rb_filepath_tree_t, &(ht_ctx->paths)) {
        printf("path: %s\n", hta_filepath->path);
    }

    return;
}
