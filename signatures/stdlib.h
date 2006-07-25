void *malloc(unsigned int size);
void free(void* ptr);
void exit(int code);

typedef int main(int argc, char **argv, char **envp);

int stat(const char* filename, struct stat* st);

void abort(void);

typedef void *va_list;
// exported by msvcrt.dll
void _assert(const char *cond, const char *file, int errcode);
int _vsnprintf(char[] *buf, size_t size, const char *format, va_list ap);
