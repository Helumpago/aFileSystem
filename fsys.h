#ifndef __FSYS_H__
#define __FSYS_H__

#define MAX_DESC 32 // Maximum number of concurrent file descriptors allowed

/**
 * Describes a single file descriptor
 */
struct fildes {

};

/**
 * Contains the mapping from integers to file descriptors
 * Supports 32 concurrent file descriptors
 */
struct fildes_table {
	struct fildes* fds[MAX_DESC]; // Reference to all open file descriptors
	int num_open; // Number of open descriptors.
};

int make_fs(char* disk_name);
int mount_fs(char* disk_name);
int umount_fs(char* disk_name);
int fs_open(char* name);
int fs_close(int fildes);
int fs_create(char* name);
int fs_delete(char* name);
int fs_read(int fildes, void* buf, size_t nbyte);
int fs_write(int fildes, void* buf, size_t nbyte);
int fs_get_fileszie(int fildes);
int fs_lseek(int fildes, off_t offset);
int fs_truncate(int fildes, off_t length);
void print_block(int block);

#endif