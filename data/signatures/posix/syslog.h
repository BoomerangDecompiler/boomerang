
void  closelog(void);
void  openlog(const char *ident, int logopt, int facility);
int   setlogmask(int maskpri);
void  syslog(int priority, const char *message, ...);
