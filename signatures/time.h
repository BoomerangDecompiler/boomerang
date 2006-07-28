
typedef int time_t;

time_t time(time_t *__timer);

typedef struct {
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

char *ctime(const time_t *clock);
double difftime(time_t time1, time_t time0);
char *asctime(const tm *tm);
tm *localtime(const time_t *clock);
tm *gmtime(const time_t *clock);
time_t mktime(tm *tm);
time_t timegm(tm *tm);
char *ctime_r(const time_t *clock, char *buf);
tm *localtime_r(const time_t *clock, tm *result);
tm *gmtime_r(const time_t *clock, tm *result);
char *asctime_r(const tm *tm, char *buf);
size_t strftime(char *buf, size_t maxsize, const char *format, const tm *timeptr);

