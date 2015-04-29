
#include <stdlib.h>
#include "linkedlist.h"

/*
 * Initialize a new block set node
 * @return Reference to the new block set
 */
struct block_set* new_blst() {
	struct block_set* nblst = malloc(sizeof(struct block_set));
	nblst->is_alloc = 0;
	nblst->num_blocks = 0;
	nblst->offset = 0;
	nblst->next = NULL;

	return nblst;
}

/*
 * Insert a new node after the block pointed to by the given block offset
 * @param new: Block set to be inserted
 * @param offset: Offset of the block set that this block should be inserted after
 * 
 * @return 0 if successful, 
 */
int insert_blst(struct block_set* bls) {
	return -1;
}

/*
 * Release the block set pointed to by the given offset
 * @param offset: Offset of the first block in the block set to be released
 */
int release_blst(off_t offset) {
	return -1;
}