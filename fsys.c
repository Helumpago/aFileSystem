
#include <stdio.h>
#include <stddef.h>
#include <sys/types.h>
#include <string.h>
#include "fsys.h"
#include "disk.h"
#include "dir.h"

#define DATASTART DISK_BLOCKS / 2 // The start of the actual data. Everything before this is "reserved"
#define METABL 0 // Block ID for the meta block
#define SMALLBUFF 256 // Size for various I/O buffers
#define FREESTR "-1" // If a block starts with this symbol, that block is free.
#define MAX_KV_SIZE 15 + 4 + 2 // Max size of a file key-value mapping in the metadata. 15 characters for file name, 4 characters for offset, 2 characters for delimiters

#define isfree(buff) strcmp(buff, FREESTR) == 0

/* Error codes */
#define NO_BLOCKS -1 // There are no blocks available on the disk
#define SO_MUCH_FILE -2 // There are too many files alread in the system; can't create another one.

off_t get_free_bl();

/*
	 * Creates a new filesystem virtual disk
 * @param disk_name: Name of the new disk (i.e., Linux system filename)
 */
int make_fs(char* disk_name) {
	char buff[BLOCK_SIZE] = FREESTR;

	make_disk(disk_name);
	open_disk(disk_name);

	/// Set all free blocks
	int i = DATASTART; // Start setting at block 4096; Everything before that is "reserved" for meta
	for(; i < DISK_BLOCKS; i++) {
		block_write(i, buff);
	}

	close_disk();
	return 0;
}

/*
 * Prepares the given disk for use
 * @param disk_name: Path to the disk to mount
 */
int mount_fs(char* disk_name) {
	open_disk(disk_name);

	return 0;
}

/*
 * Unmounts the given disk
 * @param disk_name: Disk to unmount
 */
int umount_fs(char* disk_name) {
	close_disk();

	return -1;
}

/*
 * Opens the given virtual file
 * @param name: Name of the file to open
 */
int fs_open(char* name) {
	return -1;
}

/*
 * Closes the given virtual file
 * @param fildes: File descriptor that should be closed
 */
int fs_close(int fildes) {
	return -1;
}

/*
 * Creates a new file
 * @param name: Name of the new file
 */
int fs_create(char* name) {
	char buff[BLOCK_SIZE];

	/// Get and initialize a block
	off_t block = get_free_bl();
	printf("Putting file in %ld\n", block);
	strcpy(buff, "0");
	block_write(block, buff); // Set block as allocated
	print_block(block);

	/// Insert new file record at the end of file list
	if(BLOCK_SIZE - strlen(buff) <= MAX_KV_SIZE) // If no more room for another file-offset mapping, can't store another file.
		return SO_MUCH_FILE;

	block_read(0, buff);
	sprintf(buff, "%s%s:%ld;", buff, name, block);
	printf("New metaline: %s\n", buff);
	block_write(0, buff);

	return 0;
}

/*
 * Deletes the given file
 * @param name: File to delete
 */
int fs_delete(char* name) {
	return -1;
}

/*
 * Read from the given file
 * @param fildes: File descriptor to read from 
 * @param buf: Buffer to write to
 * @param nbyte: number of bytes to read
 * 
 * @return If no error, returns the number of bytes read. 
 * 			Returns negative if there was an error.
 */
int fs_read(int fildes, void* buf, size_t nbyte) {
	return -1;
}

/*
 * Write to the given file
 * @param fildes: File descriptor to write to 
 * @param buf: Buffer containing data to write
 * @param nbyte: number of bytes to write
 */
int fs_write(int fildes, void* buf, size_t nbyte) {
	return -1;
}

/*
 * Get the size of the given file
 * @param fildes: File descriptor
 */
int fs_get_fileszie(int fildes) {
	return -1;
}

/*
 * Seek to an offset from the start of the file
 * @param fildes: File descriptor
 * @param offset: New location from start of file
 */
int fs_lseek(int fildes, off_t offset) {
	return -1;
}

/*
 * Truncates the given file 
 * @param fildes: File descriptor
 * @param length: New length for the truncated file
 */
int fs_truncate(int fildes, off_t length) {
	return -1;
}

/**
 * Print out the contents of a specific block
 */
void print_block(int block) {
	char buff[BLOCK_SIZE];
	block_read(block, buff);
	printf("Block %d: %s\n", block, buff);
}

/************ Helper functions *******************/

/**
 * Searches through the disk for the first available block
 */
off_t get_free_bl() {
	char buff[BLOCK_SIZE];
	int i = DATASTART;
	for(; i < DISK_BLOCKS; i++) {
		block_read(i, buff);
		if(isfree(buff))
			return i;
	}

	return NO_BLOCKS;
}