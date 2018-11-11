
struct timeb
{
    time_t         time;
    unsigned short millitm;
    short          timezone;
    short          dstflag;
};

int   ftime(struct timeb *tp);
