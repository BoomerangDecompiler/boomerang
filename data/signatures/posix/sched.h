
struct sched_param
{
    int    sched_priority;
};


int    sched_get_priority_max(int policy);
int    sched_get_priority_min(int policy);
int    sched_getparam(pid_t pid, struct sched_param *param);
int    sched_getscheduler(pid_t pid);
int    sched_rr_get_interval(pid_t pid, struct timespec *interval);
int    sched_setparam(pid_t pid, const struct sched_param *param);
int    sched_setscheduler(pid_t pid, int policy, const struct sched_param *param);
int    sched_yield(void);
