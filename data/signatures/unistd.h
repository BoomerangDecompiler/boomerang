int getopt(int argc, char *argv[], const char *optstring);

typedef int off_t;

off_t lseek(int fildes, off_t offset, int whence);
int close(int d);
