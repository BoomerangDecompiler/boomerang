int printf(char *fmt, ...);
int scanf(char *fmt, ...);
char *getenv(char *name);
int fread(char *ptr, int size, int nitems, FILE *stream);
int fwrite(char *ptr, int size, int nitems, FILE *stream);
int fclose(FILE *stream);
int puts(char *s);
void perror(const char *s);
