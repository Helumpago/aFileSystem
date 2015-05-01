
#include <stdlib.h>
#include "disk.h"
#include "dir.h"

/**
 * Initializes a new directory
 */
struct dir* create_dir(int num_blocks) {
	struct dir* new = malloc(sizeof(struct dir));
	new->num_blocks = num_blocks;
	new->last_alloc = 0;
	new->free_pool = NULL;

	return new;
}

/**
 * Creates a new file in the currently open directory
 */
void create_file(char* name)
{

}