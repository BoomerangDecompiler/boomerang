

int       msgctl(int msqid, int cmd, struct msqid_ds *buf);
int       msgget(key_t key, int msgflg);
ssize_t   msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg);
int       msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);
