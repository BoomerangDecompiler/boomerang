
typedef struct _FTW FTW;

int ftw(const char *path, int (*fn)(const char *path, const stat *ptr, int flag), int ndirs);
int nftw(const char *path, int (*fn)(const char *path, const stat *ptr, int flag, FTW *ftw), int depth, int flags);
