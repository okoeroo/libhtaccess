#ifndef HTACCESS_INTERNAL_H
    #define HTACCESS_INTERNAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "htaccess/queue.h"
#include "htaccess/tree.h"

typedef enum htaccess_directives_e {
    NO_DIRECTIVE = 0,
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

typedef enum directive_quotation_e {
    HTA_MUST_QUOTE,
    HTA_OPT_QUOTE,
    HTA_NO_QUOTE
} directive_quotation_t;

typedef struct rb_directive_map_s {
    RB_ENTRY(rb_directive_map_s) entry;

    htaccess_directive_type_t type;
    const char *str;
    unsigned int len;
    unsigned short par_cnt;
    directive_quotation_t quotation;
} htaccess_directive_map_t;

int htaccess_directive_map_cmp(htaccess_directive_map_t *, htaccess_directive_map_t *);
RB_HEAD(directive_map_tree_t, rb_directive_map_s);
RB_PROTOTYPE(directive_map_tree_t, rb_directive_map_s, entry, htaccess_directive_map_cmp)

void directive_map_list_init(void);
htaccess_directive_map_t *search_directive_map_on_str(const char *);
htaccess_directive_map_t *search_directive_map_on_type(htaccess_directive_type_t);


typedef struct tq_directive_value_s {
    TAILQ_ENTRY(tq_directive_value_s) next;
    unsigned short v_loc;
    char *value;
} htaccess_directive_value_t;

typedef struct rb_directive_kv_s {
    RB_ENTRY(rb_directive_kv_s) next;

    htaccess_directive_map_t *key;
    TAILQ_HEAD(tq_directive_value_tree_s, tq_directive_value_s) values;
} htaccess_directive_kv_t;
int htaccess_directive_kv_cmp(struct rb_directive_kv_s *,
                              struct rb_directive_kv_s *);
RB_HEAD(rb_directive_kv_list_head_t, rb_directive_kv_s);
RB_PROTOTYPE(rb_directive_kv_list_head_t, rb_directive_kv_s, next, htaccess_directive_kv_cmp)

typedef struct rb_file_s {
    RB_ENTRY(rb_file_s) next;

    struct rb_directive_kv_list_head_t directives;

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
    size_t lineno;
    size_t indent;
    struct rb_directory_list_head_t directories;
} htaccess_ctx_t;


htaccess_directive_value_t *new_htaccess_directive_value(char *, unsigned short);
htaccess_directive_kv_t *new_htaccess_directive_kv(htaccess_directive_map_t *);
htaccess_file_t      *new_htaccess_file(void);
htaccess_directory_t *new_htaccess_directory(void);
htaccess_ctx_t       *new_htaccess_ctx(void);
void free_htaccess_directive_value(htaccess_directive_value_t *);
void free_htaccess_directive_kv(htaccess_directive_kv_t *);
void free_htaccess_file(htaccess_file_t *);
void free_htaccess_directory(htaccess_directory_t *);
void free_htaccess_ctx(htaccess_ctx_t *);


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

char *htaccess_parse_quoted_string(const char *);
int htaccess_count_token(const char *, const char *);
char *htaccess_str_returned_upto_EOL(const char *);
char *htaccess_copy_string(const char *);

int htaccess_parse_directives(const char *, htaccess_file_t *);
int htaccess_parse_files(const char *, htaccess_directory_t *);
int htaccess_parse_directory(const char *, htaccess_ctx_t *);

void htaccess_print_ctx(htaccess_ctx_t *);

#endif /* HTACCESS_INTERNAL_H */

