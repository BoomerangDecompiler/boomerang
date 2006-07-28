typedef struct {
    short e_termination;
    short e_exit; 
 } exit_status;

typedef struct {
  int tv_sec;
  int tv_usec;
} timeval;

typedef struct {
  short ut_type;
  pid_t ut_pid;
  char ut_line[32];
  char ut_id[4];
  char ut_user[32];
  char ut_host[256];
  exit_status ut_exit;
  int ut_session;
  timeval ut_tv;
  int ut_addr_v6[4];
  char __unused[20];
} utmp;

utmp *getutent(void);
utmp *getutid(const utmp *id);
utmp *getutline(const utmp *line);
utmp *pututline(const utmp *utmp);
void setutent(void);
void endutent(void);
int utmpname(const char *file);

