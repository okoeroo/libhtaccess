#ifndef HTACCESS_TYPES_H
    #define HTACCESS_TYPES_H

#include <stddef.h>
#include "htaccess/queue.h"
#include "htaccess/tree.h"

#define HTACCESS_MAX_ERROR_STR 1024

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

typedef struct rb_htpasswd_s {
    RB_ENTRY(rb_htpasswd_s) entry;

    char *username;
    char *pwhash;
} htaccess_htpasswd_t;
RB_HEAD(rb_htpasswd_tree_t, rb_htpasswd_s);

typedef struct rb_htgroup_s {
    RB_ENTRY(rb_htgroup_s) entry;

    char *groupname;
    char *username;
} htaccess_htgroup_t;
RB_HEAD(rb_htgroup_tree_t, rb_htgroup_s);

typedef struct rb_filepath_s {
    RB_ENTRY(rb_filepath_s) entry;

    unsigned short done;
    char *path; /* Never allocated! */
    htaccess_directive_type_t type;

    struct rb_htpasswd_tree_t htpasswd;
    struct rb_htgroup_tree_t  htgroup;
} htaccess_filepath_t;
RB_HEAD(rb_filepath_tree_t, rb_filepath_s);

typedef struct tq_directive_value_s {
    TAILQ_ENTRY(tq_directive_value_s) next;
    unsigned short v_loc;
    char *value;

    htaccess_filepath_t *filepath;
} htaccess_directive_value_t;

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

typedef struct rb_directive_kv_s {
    RB_ENTRY(rb_directive_kv_s) next;

    htaccess_directive_map_t *key;
    TAILQ_HEAD(tq_directive_value_tree_s, tq_directive_value_s) values;
} htaccess_directive_kv_t;
RB_HEAD(rb_directive_kv_list_head_t, rb_directive_kv_s);

typedef struct rb_file_s {
    RB_ENTRY(rb_file_s) next;

    struct rb_directive_kv_list_head_t directives;

    char *filename;
} htaccess_file_t;
RB_HEAD(rb_file_list_head_t, rb_file_s);

typedef struct rb_directory_s {
    RB_ENTRY(rb_directory_s) next;

    char *dirname;
    struct rb_file_list_head_t files;
} htaccess_directory_t;
RB_HEAD(rb_directory_list_head_t, rb_directory_s);

typedef struct htaccess_ctx_s {
    size_t lineno;
    size_t indent;

    char *error;

    struct rb_directory_list_head_t directories;
    struct rb_filepath_tree_t paths;
} htaccess_ctx_t;

#endif /* HTACCESS_TYPES_H */
