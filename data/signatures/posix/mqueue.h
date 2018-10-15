
typedef struct _mqd_t mqd_t;

struct mq_attr
{
    long    mq_flags;
    long    mq_maxmsg;
    long    mq_msgsize;
    long    mq_curmsgs;
};


int      mq_close(mqd_t mqdes);
int      mq_getattr(mqd_t mqdest, mq_attr *mqstat);
int      mq_notify(mqd_t mqdes, const struct sigevent *notification);
mqd_t    mq_open(const char *name, int oflag, ...);
ssize_t  mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio);
int      mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio);
int      mq_setattr(mqd_t mqdes, const struct mq_attr *mqstat, struct mq_attr *omqstat);
int      mq_unlink(const char *name);
