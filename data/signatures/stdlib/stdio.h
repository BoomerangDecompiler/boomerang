typedef struct _FILE FILE;
typedef unsigned long fpos_t;
typedef unsigned int size_t;
typedef void *va_list;

FILE *stdin;
FILE *stdout;
FILE *stderr;

// operations on files
int remove(const char *filename);
int rename(const char *oldname, const char *newname);
FILE *tmpfile(void);
char *tmpnam(char *str);

// file access
int fclose(FILE *stream);
int fflush(FILE *stream);
FILE *fopen(const char *filename, const char *mode );
FILE *freopen(const char *filename, const char *mode, FILE *stream);
void setbuf(FILE *stream, char *buffer);
int setvbuf(FILE *stream, char *buffer, int mode, size_t size);

// formatted input/output
int fprintf(FILE *stream, const char *format, ...);
int fscanf(FILE *stream, const char *format, ...);
int printf(const char *format, ...);
int scanf(const char *format, ...);
int snprintf(char *s, size_t n, const char *format, ...);
int sprintf(char *str, const char *format, ...);
int sscanf(const char *s, const char *format, ...);
int vfprintf(FILE *stream, const char *format, va_list arg);
int vfscanf(FILE *stream, const char *format, va_list arg);
int vprintf(const char *format, va_list arg);
int vscanf(const char *format, va_list arg);
int vsnprintf (char *s, size_t n, const char *format, va_list arg);
int vsprintf(char *s, const char *format, va_list arg);
int vsscanf(const char *s, const char *format, va_list arg);

// Character input/output
int fgetc(FILE *stream);
char *fgets(char *str, int num, FILE *stream);
int fputc(int character, FILE *stream);
int fputs(const char *str, FILE *stream);
int getc(FILE *stream);
int getchar(void);
char *gets(char *str);
int putc(int character, FILE *stream);
int putchar(int character);
int puts(const char *str);
int ungetc(int character, FILE *stream);

// Direct input/output
size_t fread(void *ptr, size_t size, size_t count, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream);

// file positioning
int fgetpos(FILE *stream, fpos_t *pos);
int fseek(FILE *stream, long int offset, int origin);
int fsetpos(FILE *stream, const fpos_t *pos);
long ftell(FILE *stream);
void rewind(FILE *stream);

// error handling
void clearerr(FILE *stream);
int feof(FILE *stream);
int ferror(FILE *stream);
void perror(const char *str);
