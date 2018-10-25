
int    chmod(const char *path, mode_t mode);
int    fchmod(int fildes, mode_t mode);
int    fstat(int fildes, struct stat *buf);
int    lstat(const char *path, struct stat *buf);
int    mkdir(const char *path, mode_t mode);
int    mkfifo(const char *path, mode_t mode);
int    mknod(const char *path, mode_t mode, dev_t dev);
int    stat(const char *path, struct stat *buf);
mode_t umask(mode_t cmask);
