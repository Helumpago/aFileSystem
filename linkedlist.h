
#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

#include <sys/types.h>

/**
 * In-memory linked-list that represents the state of the disk.
 * Each node represents a "block set". Each block set can be either
 * free or already allocated to a file. Each node holds the offset 
 * of the start of the first block in the block set.
 */
struct block_set {
	char is_alloc; // 0 if free, 1 if allocated
	char num_blocks; // Number of blocks in this block set
	off_t offset; // Offset from the start of the disk for this block

	struct block_set* next;
};

struct block_set* new_blst(); // Initialize a new block set node
int insert_blst(struct block_set* bls); // Inserts a new node into the list based off its file offset
int release_blst(off_t offset); // Free the block set pointed to by the given offset

#endif