void *malloc(unsigned int size);
void free(void* ptr);
void exit(int code);

typedef int main(int argc, char **argv, char **envp);

int stat(const char* filename, struct stat* st);

void abort(void);

// exported by msvcrt.dll
void _assert(int cond);// unknown library proc: ftell
int _vsnprintf(char *str, size_t size, const char *format, va_list ap);
