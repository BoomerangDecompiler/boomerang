typedef unsigned int size_t;
typedef int (*atexitfunc)(void);
typedef void (*atquickexitfunc)(void);
typedef int (*comparfunc)(const void *lhs, const void *rhs);

typedef struct {
    int quot;
    int rem;
} div_t;

typedef struct {
    long quot;
    long rem;
} ldiv_t;

typedef struct {
    long long quot;
    long long rem;
} lldiv_t;



// string conversion
double atof(const char *str);
int atoi(const char *str);
long atol(const char *str);
long long atoll(const char *str);
double strtod(const char *str, char **endptr);
float strtof(const char *str, char **endptr);
long strtol(const char *str, char **endptr, int base);
// long double strtold(const char *str, char **endptr);
long long strtoll(const char *str, char **endptr, int base);
unsigned long strtoul (const char* str, char** endptr, int base);
unsigned long long strtoull (const char* str, char** endptr, int base);

// pseudo-random sequence generation
int rand(void);
void srand(unsigned int seed);

// dynamic memory management
void *calloc(size_t num, size_t size);
void free(void *ptr);
void *malloc(size_t size);
void *realloc(void *ptr, size_t size);

// environment
void abort(void);
int atexit(atexitfunc func);
int __cxa_atexit(atexitfunc func, void *arg, void *dso_handle);
int at_quick_exit(atquickexitfunc func);
void exit(int status);
char *getenv(const char *name);
void quick_exit(int status);
int system(const char *command);
void _Exit(int status);

// searching and sorting
void *bsearch(const void *key, const void *base,
              size_t num, size_t size,
              comparfunc compar);
void qsort(void *base, size_t num, size_t size,
           comparfunc compar);

// int arithmetic
int abs(int n);
div_t div(int numer, int denom);
long labs (long n);
ldiv_t ldiv(long numer, long denom);
long long llabs (long long n);
lldiv_t lldiv(long long numer, long long denom);

// multi-byte characters
int mblen(const char *pmb, size_t max);
int mbtowc(wchar_t *pwc, const char *pmb, size_t max);
int wctomb(char *pmb, wchar_t wc);

// multi-byte strings
size_t mbstowcs(wchar_t *dest, const char *src, size_t max);
size_t wcstombs(char *dest, const wchar_t *src, size_t max);
