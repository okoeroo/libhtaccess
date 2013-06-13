#include "htaccess/htaccess.h"
#include "htaccess_internal.h"


int
htaccess_parse_directives(const char *buf, int *lineno, htaccess_file_t *hta_file) {
    unsigned int i, j;
    char *str;
    htaccess_directive_map_t *hta_dir_map;
    htaccess_directive_kv_t *hta_dir_kv;
    htaccess_directive_value_t *hta_dir_val;

    for (i = 0; i < strlen(buf); i++) {
        if (buf[i] == '\n') {
            (*lineno)++;
            continue;
        } else if (buf[i] == ' ' || buf[i] == '\t') {
            continue;
        } else if (strncasecmp("</files>", &buf[i], strlen("</files>")) == 0) {
            /* Found parent, backstep by one and return */
            /* printf("Found </files>, returning - %d\n", *lineno); */
            return i - 1;
        } else if ((hta_dir_map = search_directive_map_on_str(&buf[i]))) {
            /* Based on the type of dir_map, track how many parameters it expects
             * and push the parsed data into the right type of struct and tree/list
             */
            /* printf("------------ I found a \"%s\", par_cnt: %d----------------------\n", hta_dir_map->str, hta_dir_map->par_cnt); */
            hta_dir_kv = new_htaccess_directive_kv(hta_dir_map);
            if (!hta_dir_kv)
                return -1;

            /* Move beyond the matched tag */
            /* i += hta_dir_map->len; */

            /* Parse and extract the parameter count based on the directive */
            /* for (j = 0; j < hta_dir_map->par_cnt; j++) { */

            for (i += hta_dir_map->len; buf[i] != '\n'; i++) {
                /* Move beyond the whitespace */
                if ((buf[i] == '\t') || (buf[i] == ' ')) {
                    continue;
                }

                if (buf[i] == '\"') {
                    str = htaccess_parse_quoted_string(&buf[i]);
                    i += strlen(str) + 2 - 1;
                } else {
                    str = htaccess_copy_string(&buf[i]);
                    i += strlen(str) - 1;
                }

                hta_dir_val = new_htaccess_directive_value(str, 0);
                if (!hta_dir_val) {
                    free_htaccess_directive_kv(hta_dir_kv);
                    return -1;
                }
                TAILQ_INSERT_TAIL(&(hta_dir_kv->values), hta_dir_val, next);
            }
            RB_INSERT(rb_directive_kv_list_head_t, &(hta_file->directives), hta_dir_kv);
            continue;
        } else {
            printf("Unknown directive\n");
            printf("---->%.70s<----\n", &buf[i]);

            return -1;
        }
    }
    return i;
}


int
htaccess_parse_files(const char *buf, int *lineno, int *indent, htaccess_directory_t *hta_dir) {
    unsigned int i;
    int rc;
    enum parser_state_e state = NONE;
    char *str;
    htaccess_file_t *hta_file;

    for (i = 0; i < strlen(buf); i++) {
        if (buf[i] == '\n') {
            (*lineno)++;
            continue;
        } else if (buf[i] == ' ' || buf[i] == '\t') {
            continue;
        } else if (strncasecmp("</directory>", &buf[i], strlen("</directory>")) == 0) {
            /* Found parent, backstep by one and return */
            /* printf("Found </Directory> (in files), returning - %d\n", *lineno); */
            return i - 1;
        } else if (strncasecmp("</files>", &buf[i], strlen("</files>")) == 0) {
            i += strlen("</files>");
            (*indent)--;
            state = NONE;

            /* printf("RB_INSERT(rb_file_list_head_t, &(hta_dir->files), hta_file);\n"); */
            if (!hta_file) {
                printf("FAIL! no hta_dir\n");
                return -1;
            }
            RB_INSERT(rb_file_list_head_t, &(hta_dir->files), hta_file);
            hta_file = NULL;

            /* printf("State change to NONE (files) - %d\n", *lineno); */

            continue;

        } else if (strncasecmp("<files", &buf[i], strlen("<files")) == 0) {
            i += strlen("<files");
            (*indent)++;
            state = IN_TAG_FILES;
            /* printf("State change to IN_TAG_FILES - %d\n", *lineno); */

            hta_file = new_htaccess_file();
            if (!hta_file) {
                printf("Unable to construct a new file entry\n");
                return -1;
            }

            continue;
        } else if (state == IN_TAG_FILES && buf[i] == '>') {
            state = IN_CTX_FILES;
            /* printf("State change to IN_CTX_FILES - %d\n", *lineno); */
            continue;
        } else if (state == IN_TAG_FILES && buf[i] == '\"') {
            str = htaccess_parse_quoted_string(&buf[i]);
            /* printf("- %s - %d\n", str, *lineno); */
            i += strlen(str) + 2;

            if (!hta_file) {
                printf("Unable to find a file entry\n");
                return -1;
            }
            hta_file->filename = str;

            continue;
        } else if (state == IN_CTX_FILES) {
            /* Parse directives */
            rc = htaccess_parse_directives(&buf[i], lineno, hta_file);
            if (rc < 0)
                return -1;

            i += rc;
            continue;
        } else {
            printf("Parse error on line number: %d, char is: \"%c\"\n", *lineno, buf[i]);
            return -1;
        }
    }
    return i;
}

int
htaccess_parse_directory(const char *buf, int *lineno, int *indent,
                        htaccess_ctx_t *ht_ctx) {
    unsigned int i;
    int rc;
    enum parser_state_e state = NONE;
    char *str;
    htaccess_directory_t *hta_dir;


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

            /* printf("RB_INSERT(rb_directory_list_head_t, &(ht_ctx->directories), dir)\n"); */
            if (!hta_dir) {
                printf("FAIL! no hta_dir\n");
                return -1;
            }

            RB_INSERT(rb_directory_list_head_t, &(ht_ctx->directories), hta_dir);
            hta_dir = NULL;

            /* printf("State change to NONE (directory) - %d\n", *lineno); */

            continue;
        } else if (strncasecmp("<directory", &buf[i], strlen("<directory")) == 0) {
            i += strlen("<directory");
            (*indent)++;
            state = IN_TAG_DIRECTORY;
            /* printf("State change to IN_TAG_DIRECTORY - %d\n", *lineno); */

            hta_dir = new_htaccess_directory();
            if (!hta_dir) {
                printf("Unable to construct a new directory entry\n");
                return -1;
            }

            continue;
        } else if (state == IN_TAG_DIRECTORY && buf[i] == '\"') {
            str = htaccess_parse_quoted_string(&buf[i]);
            if (!hta_dir) {
                printf("Parse error: htaccess_directory_t was not created\n");
                return -1;
            }

            hta_dir->dirname = str;
            /* printf("- %s - %d\n", str, *lineno); */
            i += strlen(str) + 2;

            continue;
        } else if (state == IN_TAG_DIRECTORY && buf[i] == '>') {
            state = IN_CTX_DIRECTORY;
            /* printf("State change to IN_CTX_DIRECTORY - %d\n", *lineno); */
            continue;
        } else if (state == IN_CTX_DIRECTORY) {
            /* Parse the Files */
            rc = htaccess_parse_files(&buf[i], lineno, indent, hta_dir);
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

    htaccess_file_t *hta_file;
    htaccess_directive_kv_t *hta_dir_kv;
    htaccess_directive_value_t *hta_dir_value;

    RB_FOREACH(hta_dir, rb_directory_list_head_t, &(ht_ctx->directories)) {
        printf("hta_dir->dirname: %s\n", hta_dir->dirname);
        RB_FOREACH(hta_file, rb_file_list_head_t, &(hta_dir->files)) {
            printf("hta_file->filename: %s\n", hta_file->filename);
            RB_FOREACH(hta_dir_kv, rb_directive_kv_list_head_t, &(hta_file->directives)) {
                printf("hta_dir_kv->key->str: %s\n", hta_dir_kv->key->str);
                TAILQ_FOREACH(hta_dir_value, &(hta_dir_kv->values), next) {
                    printf("hta_dir_value->value: %s\n", hta_dir_value->value);
                }
            }
        }
    }

    return 0;
}

