#ifndef HTACCESS_INTERNAL_H
    #define HTACCESS_INTERNAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "htaccess/queue.h"
#include "htaccess/tree.h"

typedef enum htaccess_directives_e {
    AUTHNAME,
    AUTHGROUPFILE,
    REQUIRE_GROUP,
    ORDER,
    DENY_FROM,
    ALLOW_FROM,
    AUTHTYPE,
    AUTHUSERFILE,
    REQUIRE,
    REDIRECT,
    SETENVIF,
    REWRITECOND,
    REWRITERULE,
    DIRECTORYINDEX,
    ADDHANDLER,
    ERRORDOCUMENT,
    ADDDEFAULTCHARSET,
    CHARSETSOURCEENC
} htaccess_directive_type_t;

typedef struct rb_directive_s {
    RB_ENTRY(rb_directive_s) entry;

    htaccess_directive_type_t type;
    const char *str;
} htaccess_directive_t;

int htaccess_directive_cmp(htaccess_directive_t *a, htaccess_directive_t *b);
RB_HEAD(directive_tree_t, rb_directive_s);
RB_PROTOTYPE(directive_tree_t, rb_directive_s, entry, htaccess_directive_cmp)


typedef struct rb_directive_kv_s {
    RB_ENTRY(rb_directive_kv_s) next;

    htaccess_directive_type_t key;
    char *value;
} htaccess_directive_kv_t;
int htaccess_directive_kv_cmp(struct rb_directive_kv_s *,
                              struct rb_directive_kv_s *);
RB_HEAD(rb_directive_kv_list_head_t, rb_directive_kv_s);
RB_PROTOTYPE(rb_directive_kv_list_head_t, rb_directive_kv_s, next, htaccess_directive_kv_cmp)

typedef struct rb_file_s {
    RB_ENTRY(rb_file_s) next;

    struct rb_directive_kv_list_head_t directive_kvs;

    char *filename;
} htaccess_file_t;
int htaccess_file_cmp(struct rb_file_s *,
                      struct rb_file_s *);
RB_HEAD(rb_file_list_head_t, rb_file_s);
RB_PROTOTYPE(rb_file_list_head_t, rb_file_s, next, htaccess_file_cmp)

typedef struct rb_directory_s {
    RB_ENTRY(rb_directory_s) next;

    char *dirname;
    struct rb_file_list_head_t files;
} htaccess_directory_t;
int htaccess_directory_cmp(struct rb_directory_s *,
                           struct rb_directory_s *);
RB_HEAD(rb_directory_list_head_t, rb_directory_s);
RB_PROTOTYPE(rb_directory_list_head_t, rb_directory_s, next, htaccess_directory_cmp)

typedef struct htaccess_ctx_s {
    struct rb_directory_list_head_t directories;
} htaccess_ctx_t;


htaccess_file_t      *new_htaccess_file(void);
htaccess_directory_t *new_htaccess_directory(void);
htaccess_ctx_t       *new_htaccess_ctx(void);
void free_htaccess_file(htaccess_file_t *fn);
void free_htaccess_directory(htaccess_directory_t *dir);
void free_htaccess_ctx(htaccess_ctx_t *ctx);


enum parser_state_e {
    NONE,
    IN_BEGIN_TAG,
    IN_CTX_DIRECTORY,
    IN_TAG_DIRECTORY,
    IN_CTX_FILES,
    IN_TAG_FILES,
    OPENP,
    CLOSEP
};

char *htaccess_parse_quoted_string(const char *buf);
int htaccess_count_token(const char *buf, const char *tokens);
char *htaccess_str_returned_upto_EOL(const char *buf);

int htaccess_parse_directives(const char *buf, int *lineno, htaccess_file_t *hta_file);
int htaccess_parse_files(const char *buf, int *lineno, int *indent, htaccess_directory_t *dir);
int htaccess_parse_directory(const char *buf, int *lineno, int *indent, htaccess_ctx_t *ht_ctx);

#endif /* HTACCESS_INTERNAL_H */

