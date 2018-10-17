
pid_t  wait(int *stat_loc);
pid_t  wait3(int *stat_loc, int options, struct rusage *resource_usage);
int    waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options);
pid_t  waitpid(pid_t pid, int *stat_loc, int options);
