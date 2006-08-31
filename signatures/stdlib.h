typedef unsigned int size_t;
void *malloc(size_t size);
void free(void* ptr);
void *calloc(size_t number, size_t size);
void *realloc(void *ptr, size_t size);
void *reallocf(void *ptr, size_t size);

void exit(int code);

typedef int main(int argc, char* argv[], char* envp[]);

int stat(const char* filename, struct stat* st);

void abort(void);

typedef int comparfunc(const void *a, const void *b);

void qsort(void *base, size_t nmemb, size_t size, comparfunc *compar);
void qsort_r(void *base, size_t nmemb, size_t size, void *thunk, comparfunc *compar);
int heapsort(void *base, size_t nmemb, size_t size, comparfunc *compar);
int mergesort(void *base, size_t nmemb, size_t size, comparfunc *compar);

// exported by msvcrt.dll
void _assert(const char *cond, const char *file, int errcode);
typedef void *va_list;
int _vsnprintf(char[] *buf, size_t size @max(buf), const char *format, va_list ap);
