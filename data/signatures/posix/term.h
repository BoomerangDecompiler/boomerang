
typedef struct _TERMINAL TERMINAL;

int    del_curterm(TERMINAL *oterm);
int    putp(const char *str);
int    restartterm(char *term, int fildes, int *errret);
TERMINAL *set_curterm(TERMINAL *nterm);
int    setupterm(char *term, int fildes, int *errret);
int    tgetent(char *bp, const char *name);
int    tgetflag(char id[2]);
int    tgetnum(char id[2]);
char  *tgetstr(char id[2], char **area);
char  *tgoto(char *cap, int col, int row);
int    tigetflag(char *capname);
int    tigetnum(char *capname);
char  *tigetstr(char *capname);
char  *tparm(char *cap, long p1, long p2, long p3, long p4, long p5, long p6, long p7, long p8, long p9);
int    tputs(const char *str, int affcnt, int (*putfunc)(int ch));
