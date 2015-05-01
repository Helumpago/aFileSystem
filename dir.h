
#ifndef __DIR_H__
#define __DIR_H__

#include <sys/types.h>
#include "fat.h"

/**
 * Linked list containing file blocks. Used to represent files.
 * Contains the name of the file, as well as a pointer to the FAT
 */
struct block_lst {
	char* name;
	struct file* f;
};

/**
 * Describes the file system directory.
 * Contains the names for each file, as well as a pointer to the head of
 * the first block of each file.
 */
 struct dir {
 	int num_blocks; // Number of blocks this directory has available
 	off_t last_alloc; // File offset indicating the position of the next block to be allocated. 
 	struct block_lst* free_pool; // Head of the list containing free blocks
 	
 };

struct dir* create_dir(int num_blocks); // Create a new directory with num_blocks blocks worth of space

#endif