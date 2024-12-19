
#ifndef _vfs_FILE_H
#define _vfs_FILE_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <components/system.h>
#include <os/os.h>
#include <os/mem.h>
#include <os/str.h>
#include <sys/time.h>
#include <driver/mailbox_channel.h>
#include "rtc_bk.h"
#include "project_defs.h"
#include <limits.h>
#ifdef __cplusplus
extern "C" {
#endif
#ifndef  O_RDONLY
#define	O_RDONLY	0		/* +1 == FREAD */
#endif
#ifndef  O_WRONLY
#define	O_WRONLY	1		/* +1 == FWRITE */
#endif
#ifndef  O_RDWR
#define	O_RDWR		2		/* +1 == FREAD|FWRITE */
#endif
#ifndef  O_APPEND
#define	O_APPEND	0x0008
#endif
#ifndef  O_CREAT
#define	O_CREAT		0x0200
#endif


#ifndef O_DIRECTORY
#define O_DIRECTORY 0x200000
#endif

#ifndef STATFS
#define STATFS

struct statfs {
	unsigned long f_bsize;
	unsigned long f_blocks;
	unsigned long f_bfree;
	unsigned long f_bavail;
};

#endif

#define MAX_PATH_LEN 128

#ifndef DIRENT
#define DIRENT

#define DT_UNKNOWN 0
#define DT_REG 8
#define DT_DIR 4

struct dirent {
	uint8_t d_ino;
	uint8_t d_type;
	uint16_t d_reclen;
	char d_name[MAX_PATH_LEN];
};
typedef struct __dir DIR;
#endif

int vfs_file_open(const char *pathname, int flags);
int vfs_file_read(int fd, void *buffer, int size);
int vfs_file_write(int fd, const void *buf, int count);
int vfs_file_lseek(int fd, int offset, int whence);
int vfs_file_tell(int fd);
int vfs_file_close(int fd);


int vfs_file_fstat(int fd, struct stat *statbuf);
int vfs_file_fsync(int fd);
int vfs_file_ftruncate(int fd, off_t offset);
int vfs_file_fcntl(int fd, int cmd, void *arg);
int vfs_file_unlink(const char *pathname);
int vfs_file_stat(const char *pathname, struct stat *buf);
int vfs_file_rename(const char *oldpath, const char *newpath);

int vfs_file_mkdir(const char *pathname, mode_t mode);
int vfs_file_rmdir(const char *pathname);

DIR *vfs_file_opendir(const char *name);
int vfs_file_closedir(DIR *dirp);
struct dirent *vfs_file_readdir(DIR *dirp);
void vfs_file_seekdir(DIR *dirp, long loc);
long vfs_file_telldir(DIR *dirp);
void vfs_file_rewinddir(DIR *dirp);


int vfs_file_statfs(const char *path, struct statfs *buf);

void init_vfs_files(project_config_t *config);
void uninit_vfs_files();
void vfs_file_handle_req_rx(webrtc_cmd_t *pcmd);
void vfs_file_handle_resp_rx(webrtc_cmd_t *pcmd);
#ifdef __cplusplus
}
#endif



#endif /* _SOCKETS_H */
