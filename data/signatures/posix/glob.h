
typedef _glob_t glob_t;

int glob(const char *pattern, int flags, int(*errfunc)(const char *epath, int errno), glob_t *pglob);
void globfree(glob_t *pglob);
