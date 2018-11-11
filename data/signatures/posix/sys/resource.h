
int  getpriority(int which, id_t who);
int  getrlimit(int resource, struct rlimit *rlp);
int  getrusage(int who, struct rusage *r_usage);
int  setpriority(int which, id_t who, int nice);
int  setrlimit(int resource, const struct rlimit *rlp);
