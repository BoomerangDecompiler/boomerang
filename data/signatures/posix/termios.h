
speed_t cfgetispeed(const struct termios *termios_p);
speed_t cfgetospeed(const struct termios *termios_p);
int     cfsetispeed(struct termios *termios_p, speed_t speed);
int     cfsetospeed(struct termios *termios_p, speed_t speed);
int     tcdrain(int fildes);
int     tcflow(int fildes, int action);
int     tcflush(int fildes, int queue_selector);
int     tcgetattr(int fildes, struct termios *termios_p);
pid_t   tcgetsid(int fildes);
int     tcsendbreak(int fildes, int duration);
int     tcsetattr(int fildes, int optional_actions, const struct termios *termios_p);
