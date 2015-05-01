
/*
	Runs basic tests for the file system
*/

#include <stdio.h>
#include "fsys.h"

char* get_path(int argc, char** argv);
void mount(char* fsys_path);
int spam_files();

int main(int argc, char** argv) {
	char* fsys_path = get_path(argc, argv); // Path to the filesystem virtual disk

	printf("--- Making initial file system ---\n");
	make_fs(fsys_path);
	printf("--- File system created --- \n");
	printf("--- Mounting file system ---\n");
	mount_fs(fsys_path);
	printf("--- File system mounted --- \n");

	printf("--- Creating test files ---\n");
	printf("Created %d new files\n", spam_files());
	printf("--- New files created --- \n");

	print_block(0);

	printf("--- Unmounting filesystem ---\n");
	umount_fs(fsys_path);
	printf("--- Filesystem unmounted ---\n");

	return 0;
}

/**
 * Gets the path for the disk from to command line args
 */
char* get_path(int argc, char** argv) {
	if(argc <= 1) {
		fprintf(stderr, "Must include path to filesystem virtual disk");
		return 0;
	}

	return argv[1];
}

/**
 * Dumps files onto the filesystem until it decides it can't handle any more
 * @return Number of files created before filesystem rejected
 */
int spam_files() {
	char fname[16];
	int count = 0;

	do {
		count++;
		sprintf(fname, "t%d", count);
		printf("Creating %s\n", fname);
	} while(fs_create(fname) == 0);
	
	return count;
}