/* #include "htaccess/htaccess.h" */
#include "htaccess_internal.h"

int
htaccess_parse_buffer(htaccess_ctx_t *ht_ctx, char *buf) {
    int rc;

    if (!ht_ctx || !buf)
        return 1;

    rc = htaccess_parse_directory(ht_ctx, buf);
    if (rc < 0) {
        htaccess_add_error(ht_ctx, "Failed to parse buffer");
        return 1;
    }

    htaccess_process_ctx(ht_ctx);
    return 0;
}

int
htaccess_parse_file(htaccess_ctx_t *ht_ctx, const char *fname) {
    char *buf;

    if (!ht_ctx || !fname)
        return 1;

    buf = htaccess_readfile(fname);
    if (!buf) {
        htaccess_add_error(ht_ctx, "Failed to read the htaccess file \"%s\"", fname);
        return 1;
    }

    return htaccess_parse_buffer(ht_ctx, buf);
}

htaccess_decision_t
htaccess_approve_access(htaccess_ctx_t *ht_ctx, const char *directory,
                                                const char *file,
                                                const char *username) {
    htaccess_decision_t decision = HTA_INAPPLICABLE;

    htaccess_directory_t *found_dir, search_dir;
    htaccess_file_t *found_file, search_file;
    htaccess_directive_kv_t *found_directive, search_directive;
    htaccess_directive_value_t *hta_dir_value, *hta_dir_value_sub;

    /* htaccess_htpasswd_t *found_pw, search_pw; */
    htaccess_htgroup_t *found_gr, search_gr;

    search_dir.dirname = directory;
    found_dir = RB_FIND(rb_directory_list_head_t, &(ht_ctx->directories), &search_dir);
    if (!found_dir) {
        return HTA_INAPPLICABLE;
    }

    search_file.filename = file;
    found_file = RB_FIND(rb_file_list_head_t, &(found_dir->files), &search_file);
    if (!found_dir) {
        return HTA_INAPPLICABLE;
    }

    search_directive.key = search_directive_map_on_type(HTA_REQUIRE);
    found_directive = RB_FIND(rb_directive_kv_list_head_t, &(found_file->directives), &search_directive);
    if (!found_directive) {
        /* No Require directive, I would say a default deny is best */
        return HTA_DENY;
    }

    /* Special for the REQUIRE directive */
    TAILQ_FOREACH(hta_dir_value, &(found_directive->values), next) {
        if (!hta_dir_value->sub_directive)
            continue;
        else if (hta_dir_value->sub_directive->type == HTA_GROUP) {
            /* Means: require group, with a groupname */
            search_directive.key = search_directive_map_on_type(HTA_AUTHGROUPFILE);
            found_directive = RB_FIND(rb_directive_kv_list_head_t, &(found_file->directives), &search_directive);
            if (!found_directive) {
                /* No Require directive, I would say a default deny is best */
                return HTA_DENY;
            }

            TAILQ_FOREACH(hta_dir_value_sub, &(found_directive->values), next) {

                search_gr.groupname = hta_dir_value->value;
                search_gr.username  = username;

                if (hta_dir_value_sub->filepath) {
                    found_gr = RB_FIND(rb_htgroup_tree_t, &(hta_dir_value_sub->filepath->htgroup), &search_gr);
                    if (found_gr) {
                        return HTA_PERMIT;
                    }
                }
            }
        } else {
            printf("Not applicable directive: %s\n", hta_dir_value->sub_directive->str);
        }
    }
    return HTA_DENY;
}

