#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <htaccess/htaccess.h>


static htaccess_decision_t
run_search_test(htaccess_ctx_t *ht_ctx, const char *dir, const char *file, const char *user) {
    htaccess_decision_t rc;
    rc = htaccess_approve_access(ht_ctx, dir, file, user);
    return rc;

    printf("Using: dir \"%s\" file \"%s\" user \"%s\" ", dir, file, user);
    switch (rc) {
        case HTA_INAPPLICABLE:
            printf("decision: Inapplicable");
            break;
        case HTA_PERMIT:
            printf("decision: Permit");
            break;
        case HTA_DENY:
            printf("decision: Deny");
            break;
        default:
            printf("decision: Unknown!");
    }
    printf("\n");
    return rc;
}

int
main (int argc, char *argv[]) {
    htaccess_ctx_t *ht_ctx;
    const char *fname = NULL;
    int i,j;

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


    /* htaccess_print_ctx(ht_ctx); */


    run_search_test(ht_ctx, "/", "file", "okoeroo");


    #define TEST_SURROUND 10
    #define TEST_ROUNDS   10000000

    time_t start_time, total_time, intermediate_time;

    time(&start_time);

    for (j = 0; j < TEST_SURROUND; j++) {
        time(&intermediate_time);
        printf("Relative tot start: %lu\n", intermediate_time - start_time);

        for (i = 0; i < TEST_ROUNDS; i++) {
            if (run_search_test(ht_ctx, "/lat/corpora/archive/1839/imdi/acqui_data/ac-ESF/Info", "esf.html", "corpman") != HTA_PERMIT) {
                printf("Expected PERMIT\n");
                exit(1);
            }
        }
        printf(".");
        fflush(stdout);
    }
    time(&total_time);
    printf("Total time: %lu\n", total_time - start_time);
    printf("Cycles per seconds: %lu\n", (TEST_SURROUND * TEST_ROUNDS) / (total_time - start_time));

    free_htaccess_ctx(ht_ctx);

    return 0;
}

