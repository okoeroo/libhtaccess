#ifndef HTACCESS_H
    #define HTACCESS_H

#include <htaccess/htaccess_types.h>

htaccess_ctx_t       *new_htaccess_ctx(void);
void free_htaccess_ctx(htaccess_ctx_t *);

int htaccess_parse_buffer(htaccess_ctx_t *, char *);
int htaccess_parse_file(htaccess_ctx_t *, const char *);

void htaccess_print_ctx(htaccess_ctx_t *);

char *htaccess_get_error(htaccess_ctx_t *);

htaccess_decision_t
htaccess_approve_access(htaccess_ctx_t *ht_ctx, const char *,
                                                const char *,
                                                const char *);

#endif /* HTACCESS_H */
