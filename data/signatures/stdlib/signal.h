typedef int sig_atomic_t;

typedef void (*sigfunc_t)(int);

void signal(int sig, sigfunc_t sigfunc);
int raise(int sig);
