
typedef unsigned int sem_t;

int    sem_close(sem_t *sem);
int    sem_destroy(sem_t *sem);
int    sem_getvalue(sem_t *sem, int *sval);
int    sem_init(sem_t *sem, int pshared, unsigned int value);
sem_t *sem_open(const char *name, int oflag, ...);
int    sem_post(sem_t *sem);
int    sem_trywait(sem_t *sem);
int    sem_unlink(const char *name);
int    sem_wait(sem_t *sem);
