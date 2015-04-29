
#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

#define IS_TAKEN -1 // Requested block is already allocated to something

/**
 * In-memory singly linked-list that represents the state of the disk.
 * Each node represents a "block set". Each block set can be either
 * free or already allocated to a file. Each node holds the offset 
 * of the start of the first block in the block set.
 */
struct block_set {
	char is_alloc; // 0 if free, 1 if allocated
	char num_blocks; // Number of blocks in this block set
	int block_num; // Block number. IOW, multiplying block_num by the size of blocks should give this block's offset

	struct block_set* next;
};

struct block_set* new_blst(); // Initialize a new block set node
int insert_blst(struct block_set* head, struct block_set* bls); // Inserts a new node into the list based off its file offset
int release_blst(off_t offset); // Free the block set pointed to by the given offset

#endif