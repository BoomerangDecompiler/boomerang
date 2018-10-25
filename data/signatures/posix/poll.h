
typedef unsigned int nfds_t;

struct pollfd
{
    int   fd;
    short events;
    short revents;
};

int poll(pollfd fds[], nfds_t nfds, int timeout);
