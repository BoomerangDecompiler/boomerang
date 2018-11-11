
void *shmat(int shmid, const void *shmaddr, int shmflg);
int   shmctl(int shmid, int cmd, struct shmid_ds *buf);
int   shmdt(const void *shmaddr);
int   shmget(key_t key, size_t size, int shmflg);
