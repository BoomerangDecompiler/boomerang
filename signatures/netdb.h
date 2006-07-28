
typedef struct {
        int     ai_flags;
        int     ai_family;
        int     ai_socktype;
        int     ai_protocol;
        size_t  ai_addrlen;
        char    *ai_canonname;
        sockaddr *ai_addr;
        addrinfo *ai_next;
} addrinfo;

int getaddrinfo(const char *nodename, const char *servname, const addrinfo *hints, addrinfo **res);
void freeaddrinfo(addrinfo *ai);
char *gai_strerror(int ecode);
