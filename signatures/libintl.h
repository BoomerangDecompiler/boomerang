char *textdomain(const char *domainname);
char *bindtextdomain(const char *domainname, const char *dirname);
char *gettext (const char *msgid);
char *dgettext (const char *domainname, const char *msgid);
char *dcgettext (const char *domainname, const char *msgid, int category);

