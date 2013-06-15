#ifndef HTACCESS_H
    #define HTACCESS_H

#include <htaccess/htaccess_types.h>

int htaccess_parse_buffer(htaccess_ctx_t *, char *);
int htaccess_parse_file(htaccess_ctx_t *, const char *);
htaccess_ctx_t       *new_htaccess_ctx(void);
void free_htaccess_ctx(htaccess_ctx_t *);

char *htaccess_get_error(htaccess_ctx_t *);

#endif /* HTACCESS_H */
