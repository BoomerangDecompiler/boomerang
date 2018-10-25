
int    isastream(int fildes);
int    getmsg(int fildes, struct strbuf *ctlptr, struct strbuf *dataptr, int *flagsp);
int    getpmsg(int fildes, struct strbuf *ctlptr, struct strbuf *dataptr, int *bandp, int *flagsp);
int    ioctl(int fildes, int request, ...);
int    putmsg(int fildes, const struct strbuf *ctlptr, const struct strbuf *dataptr, int flags);
int    putpmsg(int fildes, const struct strbuf *ctlptr, const struct strbuf *dataptr, int band, int flags);
int    fattach(int fildes, const char *path);
int    fdetach(const char *path);
