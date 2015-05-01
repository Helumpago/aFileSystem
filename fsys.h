#ifndef __FSYS_H__
#define __FSYS_H__

/**
 * Contains metadata for this filesystem
 */
struct super_t {
	off_t free_blocks; // Reference to the first link in the free block list
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