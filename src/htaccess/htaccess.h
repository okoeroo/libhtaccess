#ifndef HTACCESS_H
    #define HTACCESS_H

int lhta_parse_and_load(const char *buf);
char *lhta_readfileintobuffer(const char *fname);

#endif /* HTACCESS_H */
