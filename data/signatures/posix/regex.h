
struct regex_t
{
    size_t re_nsub;
};

typedef ssize_t regoff_t;

struct regmatch_t
{
    regoff_t    rm_so;
    regoff_t    rm_eo;
};

int    regcomp(regex_t *preg, const char *pattern, int cflags);
int    regexec(const regex_t *preg, const char *string, size_t nmatch, regmatch_t pmatch[], int eflags);
size_t regerror(nt errcode, const regex_t *preg, char *errbuf, size_t errbuf_size);
void   regfree(regex_t *preg);
