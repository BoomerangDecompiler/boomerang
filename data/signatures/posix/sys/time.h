
typedef struct _fd_set fd_set;

int   getitimer(int which, struct itimerval *value);
int   setitimer(int which, const struct itimerval *value, struct itimerval *ovalue);
int   gettimeofday(struct timeval *tp, void *tzp);
int   select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *errorfds, struct timeval *timeout);
int   utimes(const char *path, const struct timeval times[2]);
