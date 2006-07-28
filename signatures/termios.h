
typedef unsigned int    tcflag_t;
typedef unsigned char   cc_t;
typedef unsigned int speed_t;

typedef struct {
        tcflag_t        c_iflag;
        tcflag_t        c_oflag;
        tcflag_t        c_cflag;
        tcflag_t        c_lflag;
        cc_t            c_cc[20];
        speed_t         c_ispeed;
        speed_t         c_ospeed;
} termios;

speed_t cfgetispeed(const termios *t);
int cfsetispeed(termios *t, speed_t speed);
speed_t cfgetospeed(const termios *t);
int cfsetospeed(termios *t, speed_t speed);
int cfsetspeed(termios *t, speed_t speed);
void cfmakeraw(termios *t);
int tcgetattr(int fd, termios *t);
int tcsetattr(int fd, int action, const termios *t);
