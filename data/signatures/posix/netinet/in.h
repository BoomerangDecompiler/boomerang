
typedef unsigned short in_port_t;
typedef unsigned int   in_addr_t;
typedef unsigned int   sa_family_t;

struct in_addr
{
    in_addr_t s_addr;
};

struct sockaddr_in
{
    sa_family_t    sin_family;
    in_port_t      sin_port;
    struct in_addr sin_addr;
    unsigned char  sin_zero[8];
};
