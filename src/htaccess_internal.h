#ifndef HTACCESS_INTERNAL_H
    #define HTACCESS_INTERNAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "htaccess/queue.h"
#include "htaccess/tree.h"


typedef struct rb_files_s {
    char *filename;
    RB_ENTRY(rb_files_s) next;
} htaccess_files_t;
int htaccess_files_cmp(struct rb_files_s *,
                       struct rb_files_s *);
RB_HEAD(rb_files_list_head_t, rb_files_s);
RB_PROTOTYPE(rb_files_list_head_t, rb_files_s, next, htaccess_files_cmp)

typedef struct rb_directory_s {
    char *dirname;
    RB_ENTRY(rb_directory_s) next;
} htaccess_directory_t;
int htaccess_directory_cmp(struct rb_directory_s *,
                           struct rb_directory_s *);
RB_HEAD(rb_directory_list_head_t, rb_directory_s);
RB_PROTOTYPE(rb_directory_list_head_t, rb_directory_s, next, htaccess_directory_cmp)

typedef struct htaccess_ctx_s {
    struct rb_directory_list_head_t directories;
} htaccess_ctx_t;


htaccess_directory_t *new_htaccess_directory(void);
htaccess_ctx_t       *new_htaccess_ctx(void);

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

int htaccess_parse_directives(const char *buf, int *lineno);
int htaccess_parse_files(const char *buf, int *lineno, int *indent, htaccess_directory_t *dir);
int htaccess_parse_directory(const char *buf, int *lineno, int *indent, htaccess_ctx_t *ht_ctx);

#endif /* HTACCESS_INTERNAL_H */

