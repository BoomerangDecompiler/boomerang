typedef struct {
        char    *pw_name; 
        char    *pw_passwd;    
        uid_t   pw_uid;   
        gid_t   pw_gid;    
        time_t  pw_change;   
        char    *pw_class;    
        char    *pw_gecos;     
        char    *pw_dir;         
        char    *pw_shell;    
        time_t  pw_expire;    
        int     pw_fields; 
} passwd;

passwd *getpwent(void);
int getpwent_r(passwd *pwd, char *buffer, size_t sz @max(buffer), passwd **result);
passwd *getpwnam(const char *login);
int getpwnam_r(const char *name, passwd *pwd, char *buffer, size_t sz @max(buffer), passwd **result);
passwd *getpwuid(uid_t uid);
int getpwuid_r(uid_t uid, passwd *pwd, char *buffer, size_t sz @max(buffer), passwd **result);
int setpassent(int stayopen);
void setpwent(void);
void endpwent(void);
