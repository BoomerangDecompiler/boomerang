
typedef struct _mbstate_t mbstate_t;
typedef unsigned int size_t;
typedef struct _FILE FILE;
typedef unsigned int wint_t;

// input/output
wint_t fgetwc(FILE *stream);
wchar_t *fgetws (wchar_t *ws, int num, FILE *stream);
wint_t fputwc(wchar_t wc, FILE *stream);
int fputws(const wchar_t *ws, FILE *stream);
int fwide(FILE *stream, int mode);
int fwprintf(FILE *stream, const wchar_t *format, ...);
int fwscanf(FILE *stream, const wchar_t *format, ...);
wint_t getwc(FILE *stream);
wint_t getwchar(void);
wint_t putwc(wchar_t wc, FILE *stream);
wint_t putwchar(wchar_t wc);
int swprintf(wchar_t *ws, size_t len, const wchar_t *format, ...);
int swscanf(const wchar_t *ws, const wchar_t *format, ...);
wint_t ungetwc(wint_t wc, FILE *stream);
int vfwprintf(FILE *stream, const wchar_t *format, va_list arg);
int vfwscanf(FILE *stream, const wchar_t *format, va_list arg);
int vswprintf(wchar_t *ws, size_t len, const wchar_t *format, va_list arg);
int vswscanf(const wchar_t *ws, const wchar_t *format, va_list arg);
int vwprintf(const wchar_t *format, va_list arg);
int vwscanf(const wchar_t *format, va_list arg);
int wprintf(const wchar_t *format, ...);
int wscanf(const wchar_t *format, ...);

// general utilities
double wcstod (const wchar_t *str, wchar_t **endptr);
float wcstof(const wchar_t *str, wchar_t **endptr);
long wcstol(const wchar_t *str, wchar_t **endptr, int base);
// long double wcstold(const wchar_t *str, wchar_t **endptr);
long long strtoll(const wchar_t* str, wchar_t** endptr, int base);
unsigned long wcstoul(const wchar_t *str, wchar_t **endptr, int base);
unsigned long long wcstoull(const wchar_t *str, wchar_t **endptr, int base);

// character/string conversion
wint_t btowc(int c);
size_t mbrlen(const char *pmb, size_t max, mbstate_t *ps);
size_t mbrtowc(wchar_t *pwc, const char *pmb, size_t max, mbstate_t *ps);
int mbsinit(const mbstate_t *ps);
size_t mbsrtowcs(wchar_t *dest, const char **src, size_t max, mbstate_t *ps);
size_t wcrtomb(char *pmb, wchar_t wc, mbstate_t *ps);
int wctob(wint_t wc);
size_t wcsrtombs(char *dest, const wchar_t **src, size_t max, mbstate_t *ps);

// strings
wchar_t *wcscat(wchar_t *destination, const wchar_t *source);
const wchar_t *wcschr(const wchar_t *ws, wchar_t wc);
wchar_t *wcschr(wchar_t *ws, wchar_t wc);
int wcscmp(const wchar_t *wcs1, const wchar_t *wcs2);
int wcscoll(const wchar_t *wcs1, const wchar_t *wcs2);
wchar_t *wcscpy(wchar_t *destination, const wchar_t *source);
size_t wcscspn(const wchar_t *wcs1, const wchar_t *wcs2);
size_t wcslen(const wchar_t *wcs);
wchar_t *wcsncat(wchar_t *destination, const wchar_t *source, size_t num);
int wcsncmp(const wchar_t *wcs1, const wchar_t *wcs2, size_t num);
wchar_t *wcsncpy(wchar_t *destination, const wchar_t *source, size_t num);
const wchar_t *wcspbrk(const wchar_t *wcs1, const wchar_t *wcs2);
wchar_t *wcspbrk(wchar_t *wcs1, const wchar_t *wcs2);
const wchar_t *wcsrchr(const wchar_t *ws, wchar_t wc);
wchar_t *wcsrchr(wchar_t *ws, wchar_t wc);
size_t wcsspn(const wchar_t *wcs1, const wchar_t *wcs2);
const wchar_t *wcsstr(const wchar_t* wcs1, const wchar_t *wcs2);
wchar_t *wcsstr(wchar_t *wcs1, const wchar_t *wcs2);
wchar_t *wcstok(wchar_t *wcs, const wchar_t *delimiters);
size_t wcsxfrm(wchar_t *destination, const wchar_t *source, size_t num);
const wchar_t *wmemchr(const wchar_t* ptr, wchar_t wc, size_t num);
wchar_t *wmemchr(wchar_t *ptr, wchar_t wc, size_t num);
int wmemcmp(const wchar_t *ptr1, const wchar_t *ptr2, size_t num);
wchar_t *wmemcpy(wchar_t *destination, const wchar_t *source, size_t num);
wchar_t *wmemmove(wchar_t *destination, const wchar_t *source, size_t num);
wchar_t *wmemset(wchar_t *ptr, wchar_t wc, size_t num);

// timeptr
size_t wcsftime (wchar_t* ptr, size_t maxsize, const wchar_t* format,
                 const struct tm* timeptr);
