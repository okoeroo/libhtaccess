#ifndef HTACCESS_H
    #define HTACCESS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "htaccess/queue.h"
#include "htaccess/tree.h"

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



char *parse_quoted_string(const char *buf);
int count_token(const char *buf, const char *tokens);
char *str_returned_upto_EOL(const char *buf);
int parse_directives(const char *buf, int *lineno);
int parse_files(const char *buf, int *lineno, int *indent);
int parse_directory(const char *buf, int *lineno, int *indent);
int parse_and_load(const char *buf);
char *readfileintobuffer(const char *fname);

#endif /* HTACCESS_H */
