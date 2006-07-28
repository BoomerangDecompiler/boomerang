
// most of these are taken from http://refspecs.freestandards.org/LSB_3.0.0/LSB-Core-generic/LSB-Core-generic/libcman.html

typedef int int32_t;

int32_t **__ctype_toupper_loc(void);
const unsigned short **__ctype_b_loc(void);
char[] *__strdup(const char[] *str);
int _IO_getc(FILE *fp);

int __lxstat(int version, char *__path, stat *__statbuf);
int __fxstat(int vers, int fd, stat *buf);
int __strtol_internal(const char *__nptr, char **__endptr, int __base, int __group);
int * __errno_location(void);

int __vsnprintf_chk(char *buf, size_t szbuf, int n, size_t m, const char *s, va_list args);
int __snprintf_chk(char *buf, size_t szbuf, int n, size_t m, const char *s, ...);

