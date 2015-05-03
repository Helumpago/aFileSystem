#ifndef __FSYS_H__
#define __FSYS_H__

#define MAX_DESC 32 // Maximum number of concurrent file descriptors allowed
#define FILENAME_SIZE 15 // Max length of a name of a file

/* Error codes */
#define NO_BLOCKS -1 // There are no blocks available on the disk
#define SO_MUCH_FILE -2 // There are too many files alread in the system; can't create another one.
#define VERY_DESCRIPTION -3 // There are too many file descriptors open
#define WOW -4 // No real reason for this.
#define BAD_FILDES -5 // Requested an invalid file descriptor
#define NO_FILE -6 // No file with that name exists
#define NAME_TOO_LARGE -7
#define FILE_EXISTS -8 // Can't create the file because a file with that name already exists
#define LSEEK_OUT_OF_BOUNDS -9 // Asked to seek past the end of the file

/**
 * Describes a single file descriptor
 */
struct fildes {
	char fname[FILENAME_SIZE];
	int blk_num; // Block the cursor is in
	off_t blk_off; // Offset from the beginning of this block
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
int fs_get_filesize(int fildes);
int fs_lseek(int fildes, off_t offset);
int fs_truncate(int fildes, off_t length);
void print_block(int block);

#endif