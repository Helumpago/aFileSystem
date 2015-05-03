
#include <stdio.h>
#include <stddef.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include "fsys.h"
#include "disk.h"

/* Constants */
#define DATASTART DISK_BLOCKS / 2 // The start of the actual data. Everything before this is "reserved"
#define METABL 0 // Block ID for the meta block
#define SMALLBUFF 256 // Size for various I/O buffers
#define FREESTR "-1" // If a block starts with this symbol, that block is free.
#define MAX_KV_SIZE 15 + 4 + 2 // Max size of a file key-value mapping in the metadata. 15 characters for file name, 4 characters for offset, 2 characters for delimiters

/* Macros */
#define isfree(buff) strcmp(buff, FREESTR) == 0

/* Error codes */
#define NO_BLOCKS -1 // There are no blocks available on the disk
#define SO_MUCH_FILE -2 // There are too many files alread in the system; can't create another one.
#define VERY_DESCRIPTION -3 // There are too many file descriptors open
#define WOW -4 // No real reason for this.
#define BAD_FILDES -5 // Requested an invalid file descriptor

/* Global variables */
struct fildes_table open_fildes; // List of open file descriptors

/* Helper functions */
off_t get_free_bl();
struct fildes* get_file(int fildes);

/* Implementation */
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
	int fd = open_fildes.num_open;
	if(fd + 1 >= MAX_DESC) {
		return VERY_DESCRIPTION;
	}

	struct fildes* new = malloc(sizeof(struct fildes));
	new->cursor = 0;
	open_fildes.fds[fd] = new;
	open_fildes.num_open++;

	return fd;
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
	off_t block = 0;
	if((block = get_free_bl()) < 0) // If the search for a free block failed, return the error code
		return block;

	strcpy(buff, "0");
	block_write(block, buff); // Set block as allocated

	/// Insert new file record at the end of file list
	block_read(0, buff);
	if(BLOCK_SIZE - strlen(buff) <= MAX_KV_SIZE) // If no more room for another file-offset mapping, can't store another file.
		return SO_MUCH_FILE;

	sprintf(buff, "%s%s:%ld;", buff, name, block);
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
	char contents[BLOCK_SIZE]; // Contents of a block

	/// Get file descriptor
	struct fildes* file = get_file(fildes);
	if(file == NULL)
		return BAD_FILDES;

	/// Write to file
	int blk_num = file->cursor / BLOCK_SIZE + 1; // The block that the cursor is in
	off_t blk_off = file->cursor % BLOCK_SIZE; // Offset from the start of this block
	
	block_read(blk_num, contents);
	contents[blk_off] = '\0'; // Mark position of cursor
	sprintf(contents, "%s%s", contents, buf);
	block_write(blk_num, contents);
	file->cursor += strlen(buf);

	print_block(blk_num);

	return strlen(buf);

	/*int nwrote = 0; // Number of bytes written
	for(; nwrote <= nbyte; buff += BLOCK_SIZE, nwrote += BLOCK_SIZE) {
		contents[blk_off] = '\0'; // Mark position of cursor
		sprintf(contents, "%s%s", contents, buf);
		if(BLOCK_SIZE - blk_off - strsize(buf) < 0) { // Not enough room in one block
			// TODO: Chunk files
		}
	}*/
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

/**
 * Returns the file that corresponds to the given file descriptor.
 * Return value NULL if fildes is invalid
 */
struct fildes* get_file(int fildes) {
	if(fildes < 0 || fildes > MAX_DESC)
		return NULL;

	return open_fildes.fds[fildes];
}