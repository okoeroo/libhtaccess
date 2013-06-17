# __libhtaccess__

### _htaccess file parsing and querying_
*****

## Author
Oscar Koeroo <okoeroo@gmail.com>

## What is it?
".htaccess files (or "distributed configuration files") provide a way to make
configuration changes on a per-directory basis. A file, containing one or more
configuration directives, is placed in a particular document directory, and the
directives apply to that directory, and all subdirectories thereof.",
http://httpd.apache.org/docs/current/howto/htaccess.html

The initial focus of this library is to implement _htaccess_ based
authorization.  The parser supports the read-in of .htpasswd and .htgroup files
and will mix these together into one queryable tree.

## ...but why?
I'm working at Nikhef (http://www.nikhef.nl) on a project for Clarin
(http://clarin.nl/) on AMS (Access rights Management System -
http://tla.mpi.nl/tools/tla-tools/ams/). The AMS system exposes the resulting
authorizations as _htaccess_ files among other options. To make the access rule
queryable a decision has been taken to integrate an XACML3 service (Like:
https://github.com/okoeroo/generalauthorization) and expose the rules via an
XACML3 (REST) query interface. The service will serve a module with a callout
interface, when a rule is hit it will query the post-parsed _htaccess_ file
with _libhtaccess_.

Also... Apache has this code build-in and interwoven in their httpd core
library, didn't feature the parse once and query from memory feature and other
parsers where script based (which will kill my potential performance).

## Current state
Work in progress, but functional and well performing

## Implements the following defacto-standards(*)
* Apache's htaccess file: http://httpd.apache.org/docs/current/howto/htaccess.html

(*) To a certain degree.

## Dependencies
None.

## Known BUGS
* Not all possible queries can be made.
* Does _not_ implement htaccess to the fullest
* Might leak memory in some cases.

## Go with the Flow
There are two types of phases, the start up phase and running each of the URI triggers.

1. Start up time
    1. Create a context: _htaccess_ctx_t *ht_ctx = new_htaccess_ctx();_
    2. Parse the htaccess file (offered as null-terminated string or filename)
        * _htaccess_parse_file(ht_ctx, "/mijn/.htaccess");_
        * _htaccess_parse_buffer(ht_ctx, buffer);_
    3. Pass the _(htaccess_ctx_t *)_ for querying.

2. Query time!
    1. _htaccess_decision_t rc = htaccess_approve_access(ht_ctx, "/var/www/html", "index.html", "okoeroo");_
        * Returns an _htaccess_decision_t_ (which is an enum), see _htaccess_decision_t_ section

3. Error and diagnostics reporting
    1. The _htaccess_get_error()_ function will return a stacktrace-like single-lined string or NULL (= nothing to report).
    2. _printf("Error: %s\n", htaccess_get_error(ht_ctx);_
    3. Printing all the things: _htaccess_print_ctx(ht_ctx);_

4. Clean up duty
    * _free_htaccess_ctx(ht_ctx);_

## htaccess_decision_t
* HTA_INAPPLICABLE: No decision to be made as there is no policy on the resource.
* HTA_PERMIT: The decision to access the resource is Permitted
* HTA_DENY: The decision to access the resource is Denied

