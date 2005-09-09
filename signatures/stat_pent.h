typedef unsigned int mode_t;
typedef unsigned long ino_t;
typedef unsigned long long dev_t;
typedef unsigned int nlink_t;
typedef unsigned int uid_t;
typedef unsigned int gid_t;
typedef long off_t;
typedef int blksize_t;
typedef int blkcnt_t;

typedef long time_t;
struct timespec {
	time_t	tv_sec;
	long	tv_nsec;
};

struct stat {
	dev_t st_dev; 
	unsigned int pad1;
	ino_t st_ino;
	mode_t st_mode;
	nlink_t st_nlink;
	uid_t st_uid;
	gid_t st_gid;
	dev_t st_rdev;
	unsigned int pad2;
	off_t st_size;
	blksize_t st_blksize;
	blkcnt_t st_blocks;
	timespec st_atime;
	timespec st_mtime;
	timespec st_ctime;
	unsigned pad4;
	unsigned pad5;
};

