
void  *dlopen(const char *file, int mode);
void  *dlsym(void *handle, const char *name);
int    dlclose(void *handle);
char  *dlerror(void);
