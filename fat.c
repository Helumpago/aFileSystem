
#include <stdlib.h>
#include "fat.h"

/*
 * Initialize a new block set node
 * @return Reference to the new block set
 */
struct fat* new_blst() {
	struct fat* nblst = malloc(sizeof(struct fat));
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
int insert_blst(struct fat* head, struct fat* bls) {
	struct fat* curr = head;
	

	return -1;
}

/*
 * Release the block set pointed to by the given offset
 * @param offset: Offset of the first block in the block set to be released
 */
int release_blst(off_t offset) {
	return -1;
}