
typedef int sa_family_t;

typedef struct {
        unsigned char   sa_len;
        sa_family_t     sa_family;
        char            sa_data[14];
} sockaddr;

typedef int socklen_t;

int getsockname(int s, sockaddr *name, socklen_t *namelen);
int socket(int domain, int type, int protocol);
int connect(int s, const sockaddr *name, socklen_t namelen);
