#ifndef HTACCESS_TYPES_H
    #define HTACCESS_TYPES_H

#include <stddef.h>
#include "htaccess/queue.h"
#include "htaccess/tree.h"

#define HTACCESS_MAX_ERROR_STR 1024

typedef enum htaccess_decision_e {
    HTA_INAPPLICABLE = 0, /* I don't have a policy about this resource in the htaccess fils */
    HTA_PERMIT,
    HTA_DENY
} htaccess_decision_t;

typedef enum htaccess_directives_e {
    HTA_NO_DIRECTIVE = 0,
    HTA_AUTHNAME,
    HTA_AUTHGROUPFILE,
    HTA_ORDER,
    HTA_DENY_FROM,
    HTA_ALLOW_FROM,
    HTA_AUTHTYPE,
    HTA_AUTHUSERFILE,
    HTA_REQUIRE,
    HTA_REDIRECT,
    HTA_SETENVIF,
    HTA_REWRITECOND,
    HTA_REWRITERULE,
    HTA_DIRECTORYINDEX,
    HTA_ADDHANDLER,
    HTA_ERRORDOCUMENT,
    HTA_ADDDEFAULTCHARSET,
    HTA_CHARSETSOURCEENC
} htaccess_directive_type_t;

typedef enum htaccess_sub_directive_type_e {
    HTA_NO_SUB_DIRECTIVE = 0,
    HTA_GROUP,
    HTA_ALL,
    HTA_ENV,
    HTA_METHOD,
    HTA_EXPR,
    HTA_USER,
    HTA_VALID_USER,
    HTA_IP
} htaccess_sub_directive_type_t;

typedef struct rb_sub_directive_map_s {
    RB_ENTRY(rb_sub_directive_map_s) entry;

    htaccess_sub_directive_type_t type;
    const char *str;
} htaccess_sub_directive_map_t;

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

    htaccess_sub_directive_map_t *sub_directive;
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

    char *filename;
    struct rb_directive_kv_list_head_t directives;
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
