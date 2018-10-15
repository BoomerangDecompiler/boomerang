
typedef unsigned int nl_catd;

int       catclose(nl_catd catd);
char     *catgets(nl_catd catd, int set_id, int msg_id, const char *s);
nl_catd   catopen(const char *name, int oflag);
