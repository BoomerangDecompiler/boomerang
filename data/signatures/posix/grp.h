
typedef struct _group group;

group         *getgrgid(gid_t gid);
group         *getgrnam(const char *name);
int            getgrgid_r(gid_t gid, group *grp, char *buffer,
                          size_t bufsize, group **result);
int            getgrnam_r(const char *name, group *grp, char *buffer,
                          size_t bufsize, group **result);
group         *getgrent(void);
void           endgrent(void);
void           setgrent(void);
