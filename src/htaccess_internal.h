#ifndef HTACCESS_INTERNAL_H
    #define HTACCESS_INTERNAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "htaccess/queue.h"
#include "htaccess/tree.h"
#include "htaccess/htaccess_types.h"


int htaccess_htpasswd_cmp(htaccess_htpasswd_t *, htaccess_htpasswd_t *);
RB_PROTOTYPE(rb_htpasswd_tree_t, rb_htpasswd_s, entry, htaccess_htpasswd_cmp)
htaccess_htpasswd_t *new_htaccess_htpasswd(void);
void free_htaccess_htpasswd(htaccess_htpasswd_t *);

int htaccess_htgroup_cmp(htaccess_htgroup_t *, htaccess_htgroup_t *);
RB_PROTOTYPE(rb_htgroup_tree_t, rb_htgroup_s, entry, htaccess_htgroup_cmp)
htaccess_htgroup_t *new_htaccess_htgroup(void);
void free_htaccess_htgroup(htaccess_htgroup_t *);

int htaccess_filepath_cmp(htaccess_filepath_t *, htaccess_filepath_t *);
RB_PROTOTYPE(rb_filepath_tree_t, rb_filepath_s, entry, htaccess_filepath_cmp)
htaccess_filepath_t *new_htaccess_filepath(void);
void free_htaccess_filepath(htaccess_filepath_t *);

int htaccess_directive_map_cmp(htaccess_directive_map_t *, htaccess_directive_map_t *);
RB_HEAD(directive_map_tree_t, rb_directive_map_s);
RB_PROTOTYPE(directive_map_tree_t, rb_directive_map_s, entry, htaccess_directive_map_cmp)
void directive_map_list_init(void);
htaccess_directive_map_t *search_directive_map_on_str(const char *);
htaccess_directive_map_t *search_directive_map_on_type(htaccess_directive_type_t);

int htaccess_sub_directive_map_cmp(htaccess_sub_directive_map_t *, htaccess_sub_directive_map_t *);
RB_HEAD(sub_directive_map_tree_t, rb_sub_directive_map_s);
RB_PROTOTYPE(sub_directive_map_tree_t, rb_sub_directive_map_s, entry, htaccess_sub_directive_map_cmp)
void sub_directive_map_list_init(void);
htaccess_sub_directive_map_t *search_sub_directive_map_on_str(const char *);
htaccess_sub_directive_map_t *search_sub_directive_map_on_type(htaccess_sub_directive_type_t);

htaccess_directive_value_t *new_htaccess_directive_value(char *, unsigned short);
void free_htaccess_directive_value(htaccess_directive_value_t *);

int htaccess_directive_kv_cmp(struct rb_directive_kv_s *, struct rb_directive_kv_s *);
RB_PROTOTYPE(rb_directive_kv_list_head_t, rb_directive_kv_s, next, htaccess_directive_kv_cmp)
htaccess_directive_kv_t *new_htaccess_directive_kv(htaccess_directive_map_t *);
void free_htaccess_directive_kv(htaccess_directive_kv_t *);

int htaccess_file_cmp(struct rb_file_s *, struct rb_file_s *);
RB_PROTOTYPE(rb_file_list_head_t, rb_file_s, next, htaccess_file_cmp)
htaccess_file_t *new_htaccess_file(void);
void free_htaccess_file(htaccess_file_t *);

int htaccess_directory_cmp(struct rb_directory_s *, struct rb_directory_s *);
RB_PROTOTYPE(rb_directory_list_head_t, rb_directory_s, next, htaccess_directory_cmp)
htaccess_directory_t *new_htaccess_directory(void);
void free_htaccess_directory(htaccess_directory_t *);

htaccess_ctx_t *new_htaccess_ctx(void);
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
char *htaccess_str_returned_upto_colon(const char *);
char *htaccess_str_returned_upto_EOL(const char *);
char *htaccess_copy_string(const char *);

int htaccess_parse_directives(htaccess_ctx_t *, const char *, htaccess_file_t *);
int htaccess_parse_files(htaccess_ctx_t *, const char *, htaccess_directory_t *);
int htaccess_parse_directory(htaccess_ctx_t *, const char *);
int htaccess_parse_htpasswd(htaccess_ctx_t *, htaccess_filepath_t *);
int htaccess_parse_htgroup(htaccess_ctx_t *, htaccess_filepath_t *);

htaccess_filepath_t *htaccess_search_filepath(htaccess_ctx_t *, char *);
htaccess_filepath_t *htaccess_add_filepath(htaccess_ctx_t *, char *);
void htaccess_process_ctx(htaccess_ctx_t *);
char *htaccess_readfile(const char *);

void htaccess_print_ctx(htaccess_ctx_t *);

/* Error string handling */
int htaccess_set_error(htaccess_ctx_t *, const char *, ...);
int htaccess_add_error(htaccess_ctx_t *, const char *, ...);
char *htaccess_get_error(htaccess_ctx_t *);
void htaccess_clear_error(htaccess_ctx_t *);

#endif /* HTACCESS_INTERNAL_H */

