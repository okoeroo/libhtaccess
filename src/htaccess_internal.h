#ifndef HTACCESS_INTERNAL_H
    #define HTACCESS_INTERNAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"
#include "tree.h"

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

char *lhta_parse_quoted_string(const char *buf);
int lhta_count_token(const char *buf, const char *tokens);
char *lhta_str_returned_upto_EOL(const char *buf);
int lhta_parse_directives(const char *buf, int *lineno);
int lhta_parse_files(const char *buf, int *lineno, int *indent);
int lhta_parse_directory(const char *buf, int *lineno, int *indent);

#endif /* HTACCESS_INTERNAL_H */

