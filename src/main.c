#include <stdio.h>
#include <htaccess/htaccess.h>


static void
run_search_test(htaccess_ctx_t *ht_ctx, const char *dir, const char *file, const char *user) {
    printf("Using: dir \"%s\" file \"%s\" user \"%s\" ", dir, file, user);
    switch (htaccess_approve_access(ht_ctx, dir, file, user)) {
        case HTA_INAPPLICABLE:
            printf("decision: Inapplicable");
            break;
        case HTA_PERMIT:
            printf("decision: Permit");
            break;
        case HTA_DENY:
            printf("decision: Permit");
            break;
        default:
            printf("decision: Unknown!");
    }
    printf("\n");
    return;
}

int
main (int argc, char *argv[]) {
    htaccess_ctx_t *ht_ctx;
    const char *fname = NULL;

    if (argc != 2) {
        printf("unknown amount of arguments, nothing to test with\n");
        return 1;
    }
    fname = argv[1];

    ht_ctx = new_htaccess_ctx();
    if (!ht_ctx)
        return 1;

    if (htaccess_parse_file(ht_ctx, fname) != 0) {
        printf("htaccess_parse_file() failed! Error: %s\n", htaccess_get_error(ht_ctx));
    }


    htaccess_print_ctx(ht_ctx);


    run_search_test(ht_ctx, "/", "file", "okoeroo");
    run_search_test(ht_ctx, "/lat/corpora/archive/1839/imdi/acqui_data/ac-ESF/Info", "esf.html", "corpman");

    free_htaccess_ctx(ht_ctx);

    return 0;
}

