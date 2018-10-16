
typedef int sig_atomic_t;
typedef struct _sigset_t sigset_t;
typedef struct sigevent sigevent;

typedef struct _sigval sigval;

typedef void (*sighandler)(int sig);
//void (*bsd_signal(int sig, sighandler handler)(int p);

int    kill(pid_t pid, int sig);
int    killpg(pid_t pid, int sig);
int    pthread_kill(pthread_t thread, int sig);
int    pthread_sigmask(int how, const sigset_t *set, sigset_t *oset);
int    raise(int sig);
int    sigaction(int sig, const struct sigaction *act, struct sigaction *oact);
int    sigaddset(sigset_t *set, int signo);
int    sigaltstack(const stack_t *ss, stack_t *oss);
int    sigdelset(sigset_t *set, int signo);
int    sigemptyset(sigset_t *set);
int    sigfillset(sigset_t *set);
int    sighold(int sig);
int    sigignore(int sig);
int    siginterrupt(int sig, int flag);
int    sigismember(const sigset_t *set, int signo);
//void   (*signal(int sig, void (*func)(int sig)))(int p);
int    sigpause(int sig);
int    sigpending(sigset_t *set);
int    sigprocmask(int how, const sigset_t *set, sigset_t *oset);
int    sigqueue(pid_t pid, int signo, const sigval value);
int    sigrelse(int sig);
//void   (*sigset(int sig, void (*disp)(int sig)))(int p);
int    sigstack(struct sigstack *ss, struct sigstack *oss);
int    sigsuspend(const sigset_t *sigmask);
int    sigtimedwait(const sigset_t *set, siginfo_t *info,
                    const struct timespec *timeout);
int    sigwait(const sigset_t *set, int *sig);
int    sigwaitinfo(const sigset_t *set, siginfo_t *info);
