

int    mlock(const void *addr, size_t len);
int    mlockall(int flags);
void  *mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off);
int    mprotect(void *addr, size_t len, int prot);
int    msync(void *addr, size_t len, int flags);
int    munlock(const void * addr, size_t len);
int    munlockall(void);
int    munmap(void *addr, size_t len);
int    shm_open(const char *name, int oflag, mode_t mode);
int    shm_unlink(const char *name);
