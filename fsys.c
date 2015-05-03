
#include <stdio.h>
#include <stddef.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include "fsys.h"
#include "disk.h"

/* Constants */
//#define DATASTART DISK_BLOCKS / 2 // The start of the actual data. Everything before this is "reserved"
#define METABL 0 // Block ID for the meta block
#define SMALLBUFF 256 // Size for various I/O buffers
#define FREESTR "-1" // If a block starts with this symbol, that block is free.
#define BLK_META_SIZE 5 // Number of reserved characters at the beginning of each block
#define MAX_KV_SIZE 15 + BLK_META_SIZE + 2 // Max size of a file key-value mapping in the metadata. 15 characters for file name, BLK_META_SIZE characters for meta, 2 characters for delimiters

/* Macros */
#define isfree(buff) strcmp(buff, FREESTR) == 0

/* Global variables */
struct fildes_table open_fildes; // List of open file descriptors

/* Helper functions */
off_t get_free_bl();
struct fildes* get_file(int fildes);
void build_block(int block_id);
int get_head(char* fname);
int get_next_blk(int init_blk);

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
	int i = 1; // Start setting at block 4096; Everything before that is "reserved" for meta
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
	/// Look for available filedescriptor slot
	int fd = 0;
	if(open_fildes.num_open >= MAX_DESC) {
		return VERY_DESCRIPTION;
	}
	for(fd = 0; fd < MAX_DESC; fd++) {
		if(open_fildes.fds[fd] == NULL)
			break;
	}

	struct fildes* new = malloc(sizeof(struct fildes));
	strncpy(new->fname, name, FILENAME_SIZE);

	/// Create descriptor
	new->blk_num = get_head(name);
	printf("get_head == %d\n", new->blk_num);
	new->blk_off = 0;
	open_fildes.fds[fd] = new;
	open_fildes.num_open++;

	return fd;
}

/*
 * Closes the given virtual file
 * @param fildes: File descriptor that should be closed
 */
int fs_close(int fildes) {
	if(fildes < 0 || fildes > MAX_DESC)
		return BAD_FILDES;

	free(open_fildes.fds[fildes]);
	open_fildes.fds[fildes] = NULL;
	open_fildes.num_open--;

	return -1;
}

/*
 * Creates a new file
 * @param name: Name of the new file
 */
int fs_create(char* name) {
	char buff[BLOCK_SIZE];

	if(strlen(name) > FILENAME_SIZE)
		return NAME_TOO_LARGE;

	/// Get and initialize a block
	off_t block = 0;
	if((block = get_free_bl()) < 0) // If the search for a free block failed, return the error code
		return block;

	/// Check whether file already exists
	if(get_head(name) != NO_FILE) {
		printf("File %s already exists!\n", name);
		return FILE_EXISTS;
	}

	/// Insert new file record at the end of file list
	block_read(0, buff);
	if(BLOCK_SIZE - strlen(buff) <= MAX_KV_SIZE) // If no more room for another file-offset mapping, can't store another file.
		return SO_MUCH_FILE;

	sprintf(buff, "%s%s:%ld;", buff, name, block);
	block_write(0, buff);

	build_block(block);

	return 0;
}

/*
 * Deletes the given file
 * @param name: File to delete
 * @return: 0 for success, -1 for failure
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
	struct fildes* file = open_fildes.fds[fildes];
	char readBuff[BLOCK_SIZE];
	char* offset = NULL;
	
	if(file == NULL)
		return BAD_FILDES;
	
	// read the block from disk
	block_read(file->blk_num, readBuff);
	
	// set the first byte to the offset
	offset = readBuff + file->blk_off + BLK_META_SIZE - 1;
	
	// if the read does not run off the block
	// return it
	if(strlen(offset) >= nbyte){
		strncpy(buf, offset, nbyte);
		return nbyte;
	}
	// otherwise, gonna need to find the next block!
	else{
		strncpy(buf, offset, strlen(offset));
		
		while((file->blk_num = get_next_blk(file->blk_num)) > 0){
			// read the block from disk
			block_read(file->blk_num, readBuff);
			// set the first byte to the offset
			offset = readBuff + file->blk_off + BLK_META_SIZE - 1;
			
			// if the end is in this block, you're done!
			if(strlen(offset) >= nbyte){
				strncpy(buf, offset, nbyte);
				return nbyte;
			}else{ // otherwise, add it to the string and go again
				strncat(buf, offset, strlen(offset));
			}
		}
	}

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
	char new_contents[BLOCK_SIZE]; // Holds the current chunk to be written
	int nwrote = 0; // Number of bytes that have been written

	/// Get file descriptor
	struct fildes* file = get_file(fildes);
	if(file == NULL)
		return BAD_FILDES;

	/// Write to file
	if(nbyte > strlen(buf))
		nbyte = strlen(buf);

	while(nwrote < nbyte) {
		/// Write new block
		block_read(file->blk_num, contents);
		contents[file->blk_off + BLK_META_SIZE - 1] = '\0';
		int block_space = BLOCK_SIZE - BLK_META_SIZE - file->blk_off;
		strncpy(new_contents, buf, block_space);
		new_contents[block_space] = '\0';
		sprintf(contents, "%s%s", contents, new_contents);
		block_write(file->blk_num, contents);

		int nwrite = (block_space > strlen(buf)) ? strlen(buf) : block_space;
		file->blk_off += nwrite;
		nwrote += nwrite;
		buf += nwrite;

		/// Allocate a new block
		if(file->blk_off >= BLOCK_SIZE - BLK_META_SIZE) {
			off_t nblock = get_free_bl();
			if(nblock < 0)
				return nwrote;

			build_block(nblock);

			/// Link the new block into the file
			block_read(file->blk_num, contents);
			sprintf(contents, "%04ld%s", nblock, contents + BLK_META_SIZE);
			block_write(file->blk_num, contents);

			file->blk_num = nblock;
			file->blk_off = 0;
		}
	}

	return nwrote;
}

/*
 * Get the number of blocks in the file
 * @param fildes: File descriptor
 */
int fs_get_filesize(int fildes) {
	struct fildes* file = get_file(fildes);
	if(file == NULL)
		return BAD_FILDES;

	int blk_count = 0;
	int curr_blk = 0;
	for(curr_blk = get_head(file->fname); curr_blk > 0; curr_blk = get_next_blk(curr_blk)) {
		printf("curr_blk == %d\n", curr_blk);
		blk_count++;
	}
	printf("After exiting, curr_blk == %d\n", curr_blk);

	return blk_count * BLOCK_SIZE;
}

/*
 * Seek to an offset from the start of the file
 * @param fildes: File descriptor
 * @param offset: New location from start of file
 */
int fs_lseek(int fildes, off_t offset) {
	struct fildes* file = get_file(fildes);
	if(file == NULL)
		return BAD_FILDES;

	int blk_stop = (offset + BLK_META_SIZE) / BLOCK_SIZE; // Get the block into which the cursor should be placed
	int off_stop = (offset + BLK_META_SIZE) % BLOCK_SIZE; // Get the offset into that block

	file->blk_num = get_head(file->fname);
	while(file->blk_num != blk_stop)
		file->blk_num = get_next_blk(file->blk_num);

	file->blk_off = off_stop;

	return 0;
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
	int i = 1;
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

/**
 * Initialize a new block
 */
void build_block(int block_id) {
	char buff[5] = "0000";
	block_write(block_id, buff);
}


/**
 * Returns the block number that is the first block in the block list
 * for the given file
 */
int get_head(char* fname) {
	char buff[BLOCK_SIZE];
	char *kv = NULL;
	int i = 0;
	int file_found = 0; // Flag indicating whether the file was found
	block_read(0, buff);
	for(kv = strtok(buff, ";"); kv != NULL; kv = strtok(NULL, ";")) {
		/// Get only the filename (key) portion of the key-value pair
		i = 0;
		while(kv[i] != ':')
			i++;
		kv[i] = '\0';

		/// File has been found!
		printf("fname == %s\tcurrname == %s\n", fname, kv);
		if(strncmp(fname, kv, FILENAME_SIZE) == 0) {
			file_found = 1;
			break;
		}
	}

	if(!file_found)
		return NO_FILE;

	return atoi(kv + i + 1);
}

/**
 * Get the next block that init_blk refers to
 */
int get_next_blk(int init_blk) {
	char buff[BLOCK_SIZE];

	block_read(init_blk, buff);
	buff[BLK_META_SIZE] = '\0';
	return atoi(buff);
}