
typedef struct _aiocb aiocb;
typedef int ssize_t;

int      aio_cancel(int fildes, aiocb *aiocbp);
int      aio_error(const aiocb *aiocbp);
int      aio_fsync(int op, aiocb *aiocbp);
int      aio_read(aiocb *aiocbp);
ssize_t  aio_return(aiocb *aiocbp);
int      aio_suspend(const aiocb *list[], int nent, const struct timespec *timeout);
int      aio_write(aiocb *aiocbp);
int      lio_listio(int mode, aiocb *list[], int nent, struct sigevent *sig);
