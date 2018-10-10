
typedef long clock_t;
typedef long time_t;
typedef unsigned int size_t;

typedef struct
{
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
} tm;

// time manipulation
clock_t clock (void);
double difftime(time_t end, time_t beginning);
time_t mktime(tm *timeptr);
time_t time(time_t *timer);

// conversion
char *asctime(const tm *timeptr);
char *ctime(const time_t *timer);
tm *gmtime(const time_t *timer);
tm *localtime(const time_t *timer);
size_t strftime(char *ptr, size_t maxsize @max(ptr), const char *format, const tm *timeptr);

