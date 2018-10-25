typedef int sig_atomic_t;

typedef void (*sigfunc_t)(int param);

void signal(int sig, sigfunc_t sigfunc);
int raise(int sig);
