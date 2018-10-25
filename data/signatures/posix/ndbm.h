
typedef struct _DBM DBM;

struct datum
{
    void *dptr;
    size_t dsize;
};

int     dbm_clearerr(DBM *db);
void    dbm_close(DBM *db);
int     dbm_delete(DBM *db, datum key);
int     dbm_error(DBM *db);
datum   dbm_fetch(DBM *db, datum key);
datum   dbm_firstkey(DBM *db);
datum   dbm_nextkey(DBM *db);
DBM    *dbm_open(const char *file, int open_flags, mode_t file_mode);
int     dbm_store(DBM *db, datum key, datum content, int store_mode);
