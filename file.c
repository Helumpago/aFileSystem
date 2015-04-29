
#include <stdlib.h>
#include "file.h"

/*
 * Initialize a new block set node
 * @return Reference to the new block set
 */
struct block_set* new_blst() {
	struct block_set* nblst = malloc(sizeof(struct block_set));
	nblst->is_alloc = 0;
	nblst->num_blocks = 0;
	nblst->block_num = 0;
	nblst->next = NULL;

	return nblst;
}

/*
 * Inserts a new node into the list. Largest fit first
 * @param head: List to insert into
 * @param new: Block set to be inserted
 * 
 * @return 0 if successful, 
 */
int insert_blst(struct block_set* head, struct block_set* bls) {
	struct block_set* curr = head;
	

	return -1;
}

/*
 * Release the block set pointed to by the given offset
 * @param offset: Offset of the first block in the block set to be released
 */
int release_blst(off_t offset) {
	return -1;
}