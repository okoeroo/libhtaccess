#include "htaccess/htaccess.h"
#include "htaccess_internal.h"


int
htaccess_parse_directives(const char *buf, int *lineno) {
    unsigned int i;
    char *str;

    for (i = 0; i < strlen(buf); i++) {
        if (buf[i] == '\n') {
            (*lineno)++;
            continue;
        } else if (buf[i] == ' ' || buf[i] == '\t') {
            continue;
        } else if (strncasecmp("</files>", &buf[i], strlen("</files>")) == 0) {
            /* Found parent, backstep by one and return */
            printf("Found </files>, returning - %d\n", *lineno);
            return i - 1;
        } else if (strncasecmp("AuthName", &buf[i], strlen("AuthName")) == 0) {
            i += strlen("AuthName");
            i += htaccess_count_token(&buf[i], " \t\n");

            str = htaccess_parse_quoted_string(&buf[i]);
            printf("-- %s - %d\n", str, *lineno);
            i += strlen(str) + 2;

            (*lineno)++;
            free(str);
            continue;

        } else if (strncasecmp("AuthGroupFile", &buf[i], strlen("AuthGroupFile")) == 0) {
            i += strlen("AuthGroupFile");
            i += htaccess_count_token(&buf[i], " \t\n");

            str = htaccess_str_returned_upto_EOL(&buf[i]);
            if (str)
                (*lineno)++;

            printf("-- %s - %d\n", str, *lineno);
            i += strlen(str);
            free(str);
            continue;
        } else if (strncasecmp("Require group", &buf[i], strlen("Require group")) == 0) {
            i += strlen("Require group");
            i += htaccess_count_token(&buf[i], " \t\n");

            str = htaccess_str_returned_upto_EOL(&buf[i]);
            if (str)
                (*lineno)++;

            printf("-- %s - %d\n", str, *lineno);
            i += strlen(str);
            free(str);
            continue;
        } else if (strncasecmp("deny from all", &buf[i], strlen("deny from all")) == 0) {
            i += strlen("deny from all");

            printf("-- deny from all - %d\n", *lineno);
            continue;
        } else {
            printf("Unknown directive\n");

            str = htaccess_str_returned_upto_EOL(&buf[i]);
            if (str)
                (*lineno)++;

            printf("-- %s - %d\n", str, *lineno);
            i += strlen(str);
            free(str);
            return -1;
        }
    }
    return i;
}


int
htaccess_parse_files(const char *buf, int *lineno, int *indent, htaccess_directory_t *dir) {
    unsigned int i;
    int rc;
    enum parser_state_e state = NONE;
    char *str;

    for (i = 0; i < strlen(buf); i++) {
        if (buf[i] == '\n') {
            (*lineno)++;
            continue;
        } else if (buf[i] == ' ' || buf[i] == '\t') {
            continue;
        } else if (strncasecmp("</directory>", &buf[i], strlen("</directory>")) == 0) {
            /* Found parent, backstep by one and return */
            printf("Found </Directory> (in files), returning - %d\n", *lineno);
            return i - 1;
        } else if (strncasecmp("</files>", &buf[i], strlen("</files>")) == 0) {
            i += strlen("</files>");
            (*indent)--;
            state = NONE;
            printf("State change to NONE (files) - %d\n", *lineno);

            continue;

        } else if (strncasecmp("<files", &buf[i], strlen("<files")) == 0) {
            i += strlen("<files");
            (*indent)++;
            state = IN_TAG_FILES;
            printf("State change to IN_TAG_FILES - %d\n", *lineno);

            continue;
        } else if (state == IN_TAG_FILES && buf[i] == '>') {
            state = IN_CTX_FILES;
            printf("State change to IN_CTX_FILES - %d\n", *lineno);
            continue;
        } else if (state == IN_TAG_FILES && buf[i] == '\"') {
            str = htaccess_parse_quoted_string(&buf[i]);
            printf("- %s - %d\n", str, *lineno);
            i += strlen(str) + 2;
            free(str);

            continue;
        } else if (state == IN_CTX_FILES) {
            /* Parse directives */
            rc = htaccess_parse_directives(&buf[i], lineno);
            if (rc < 0)
                return -1;

            i += rc;
            continue;
        } else {
            printf("Parse error on line number: %d, char is: \"%c\"\n", *lineno, buf[i]);
            return -1;
        }
    }
    printf("whut?\n");
    return i;
}

int
htaccess_parse_directory(const char *buf, int *lineno, int *indent,
                        htaccess_ctx_t *ht_ctx) {
    unsigned int i;
    int rc;
    enum parser_state_e state = NONE;
    char *str;
    htaccess_directory_t *dir;


    for (i = 0; i < strlen(buf); i++) {
        if (buf[i] == '\n') {
            (*lineno)++;
            continue;
        } else if (buf[i] == ' ' || buf[i] == '\t') {
            continue;
        } else if (strncasecmp("</directory>", &buf[i], strlen("</directory>")) == 0) {
            i += strlen("</directory>");
            (*indent)--;
            state = NONE;

            printf("RB_INSERT(rb_directory_list_head_t, &(ht_ctx->directories), dir)\n");
            RB_INSERT(rb_directory_list_head_t, &(ht_ctx->directories), dir);
            dir = NULL;

            printf("State change to NONE (directory) - %d\n", *lineno);

            continue;
        } else if (strncasecmp("<directory", &buf[i], strlen("<directory")) == 0) {
            i += strlen("<directory");
            (*indent)++;
            state = IN_TAG_DIRECTORY;
            printf("State change to IN_TAG_DIRECTORY - %d\n", *lineno);

            dir = new_htaccess_directory();
            if (!dir) {
                printf("Unable to construct a new directory entry\n");
                return -1;
            }

            continue;
        } else if (state == IN_TAG_DIRECTORY && buf[i] == '\"') {
            str = htaccess_parse_quoted_string(&buf[i]);
            printf("- %s - %d\n", str, *lineno);
            i += strlen(str) + 2;
            free(str);

            continue;
        } else if (state == IN_TAG_DIRECTORY && buf[i] == '>') {
            state = IN_CTX_DIRECTORY;
            printf("State change to IN_CTX_DIRECTORY - %d\n", *lineno);
            continue;
        } else if (state == IN_CTX_DIRECTORY) {
            /* Parse the Files */
            rc = htaccess_parse_files(&buf[i], lineno, indent, dir);
            if (rc < 0)
                return -1;

            i += rc;
            continue;
        } else {
            printf("Parse error on line number: %d\n", *lineno);
            return -1;
        }
    }
    printf("I'm done\n");

    RB_FOREACH(dir, rb_directory_list_head_t, &(ht_ctx->directories)) {
        printf("dir->dirname: %s\n", dir->dirname);
    }

    return 0;
}

