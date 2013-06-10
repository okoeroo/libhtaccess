#ifndef HTACCESS_H
    #define HTACCESS_H

int htaccess_parse_and_load(const char *buf);
char *htaccess_readfile(const char *fname);

#endif /* HTACCESS_H */
