
struct passwd
{
    char    *pw_name;
    uid_t    pw_uid;
    gid_t    pw_gid;
    char    *pw_dir;
    char    *pw_shell;
};

passwd *getpwnam(const char *name);
passwd *getpwuid(uid_t uid);
int     getpwnam_r(const char *nam, struct passwd *pwd, char *buffer,
                   size_t bufsize, struct passwd **result);
int     getpwuid_r(uid_t uid, struct passwd *pwd, char *buffer,
                   size_t bufsize, struct passwd **result);
void    endpwent(void);
passwd *getpwent(void);
void    setpwent(void);
