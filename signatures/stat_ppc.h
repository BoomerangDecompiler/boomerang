typedef unsigned int mode_t;
typedef unsigned int ino_t;
typedef unsigned long long dev_t;
typedef unsigned int nlink_t;
typedef unsigned int uid_t;
typedef unsigned int gid_t;
typedef int off_t;
typedef int time_t;
typedef int blksize_t;
typedef int blkcnt_t;

struct stat {
	dev_t st_dev;
	unsigned int pad1[2];
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
	unsigned int pad3;
	time_t st_atime;
	unsigned int pad4;
	time_t st_mtime;
	unsigned int pad5;
	time_t st_ctime;
	unsigned int pad6;
	unsigned int st_blksize;
	unsigned int st_blocks;
	char st_fstype[16];
};

