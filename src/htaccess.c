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


char *
parse_quoted_string(const char *buf, int *lineno, int *indent) {
    int i;
    char *str;
    char *buf2;

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
count_token(const char *buf, const char *tokens) {
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
str_returned_upto_EOL(const char *buf) {
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

final:
    return NULL;
}

int
parse_directives(const char *buf, int *lineno, int *indent) {
    int i;
    enum parser_state_e state = NONE;
    char *str, *str2;

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
            i += count_token(&buf[i], " \t\n");

            str = parse_quoted_string(&buf[i], lineno, indent);
            printf("-- %s - %d\n", str, *lineno);
            i += strlen(str) + 2;

            (*lineno)++;
            free(str);
            continue;

        } else if (strncasecmp("AuthGroupFile", &buf[i], strlen("AuthGroupFile")) == 0) {
            i += strlen("AuthGroupFile");
            i += count_token(&buf[i], " \t\n");

            str = str_returned_upto_EOL(&buf[i]);
            if (str)
                (*lineno)++;

            printf("-- %s - %d\n", str, *lineno);
            i += strlen(str);
            free(str);
            continue;
        } else if (strncasecmp("Require group", &buf[i], strlen("Require group")) == 0) {
            i += strlen("Require group");
            i += count_token(&buf[i], " \t\n");

            str = str_returned_upto_EOL(&buf[i]);
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

            str = str_returned_upto_EOL(&buf[i]);
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
parse_files(const char *buf, int *lineno, int *indent) {
    int i, rc;
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
            str = parse_quoted_string(&buf[i], lineno, indent);
            printf("- %s - %d\n", str, *lineno);
            i += strlen(str) + 2;
            free(str);

            continue;
        } else if (state == IN_CTX_FILES) {
            /* Parse directives */
            rc = parse_directives(&buf[i], lineno, indent);
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
parse_directory(const char *buf, int *lineno, int *indent) {
    int i, rc;
    enum parser_state_e state = NONE;
    char *str;

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
            printf("State change to NONE (directory) - %d\n", *lineno);

            continue;
        } else if (strncasecmp("<directory", &buf[i], strlen("<directory")) == 0) {
            i += strlen("<directory");
            (*indent)++;
            state = IN_TAG_DIRECTORY;
            printf("State change to IN_TAG_DIRECTORY - %d\n", *lineno);

            continue;
        } else if (state == IN_TAG_DIRECTORY && buf[i] == '\"') {
            str = parse_quoted_string(&buf[i], lineno, indent);
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
            rc = parse_files(&buf[i], lineno, indent);
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
    return 0;
}


int
parse_and_load(const char *buf) {
    int lineno = 0;
    int indent = 0;

    return parse_directory(buf, &lineno, &indent);
}

char *
readfileintobuffer(const char *fname) {
    FILE *fp;
    long size;
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
main (int argc, char *argv[]) {
    char *buf = NULL;
    const char *fname = NULL;

    fname = argv[1];

    buf = readfileintobuffer(fname);
    /* printf("\n%s\n", buf); */

    return parse_and_load(buf);
}
