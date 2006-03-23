
typedef unsigned int size_t;

int printf(char *fmt, ...);
int scanf(char *fmt, ...);
char *getenv(char *name);
FILE *fopen(const char *path, const char *mode);
int fseek(FILE *stream, long offset, int whence);
int fread(char *ptr, int size, int nitems, FILE *stream);
int fwrite(char *ptr, int size, int nitems, FILE *stream);
int fclose(FILE *stream);
int puts(char *s);
char *fgets(char[] *s, int size, FILE *stream);
void perror(const char *s);
int fprintf(FILE *stream, const char *format, ...);
int sscanf(const char *str, const char *format, ...);

int atoi(const char *nptr);

int getc_unlocked(FILE *stream);
int getc(FILE *stream);
int getchar_unlocked(void);
int putc_unlocked(int c, FILE *stream);
int putchar_unlocked(int c);

void clearerr_unlocked(FILE *stream);
int feof_unlocked(FILE *stream);
int ferror_unlocked(FILE *stream);
int fileno_unlocked(FILE *stream);
int fflush_unlocked(FILE *stream);
int fgetc_unlocked(FILE *stream);
int fputc_unlocked(int c, FILE *stream);
size_t fread_unlocked(void *ptr, size_t size, size_t n, FILE *stream);
size_t fwrite_unlocked(const void *ptr, size_t size, size_t n, FILE *stream);

char *fgets_unlocked(char *s, int n, FILE *stream);
int fputs_unlocked(const char *s, FILE *stream);


size_t read(int fd, void *buf, size_t count);
size_t write(int fd, const void *buf, size_t count);

int rename(const char *oldpath, const char *newpath);
int remove(const char *pathname);


// on windows

int _write(int fd, char buf[], int size);
