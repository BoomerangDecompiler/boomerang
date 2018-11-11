
typedef struct _DIR DIR;

int            closedir(DIR *dirp);
DIR           *opendir(const char *dirname);
struct dirent *readdir(DIR *dirp);
int            readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result);
void           rewinddir(DIR *dirp);
void           seekdir(DIR *dirp, long loc);
long           telldir(DIR *dirp);
