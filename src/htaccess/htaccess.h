#ifndef HTACCESS_H
    #define HTACCESS_H

typedef struct lhta_ctx_s {

} lhta_ctx_t;

int lhta_parse_and_load(const char *buf);
char *lhta_readfileintobuffer(const char *fname);

#endif /* HTACCESS_H */
