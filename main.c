
/*
	Runs basic tests for the file system
*/

#include <stdio.h>
#include <string.h>
#include "fsys.h"
#include "disk.h"

char* get_path(int argc, char** argv);
void mount(char* fsys_path);
int spam_files();
int open_files();

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

	printf("--- Opening files ---\n");
	printf("Opened %d files before filesystem rejected request\n", open_files());
	printf("--- Opened files ---\n");

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
	} while(fs_create(fname) == 0);
	
	return count;
}

/**
 * Opens files until the filesystem says "no".
 */
int open_files() {
	char buff[BLOCK_SIZE];
	char* kv; // Will hold the string representation of the filename-block mapping
	int count = 1;

	block_read(0, buff); // Get list of all existing files
	for(kv = strtok(buff, ";"); kv != NULL; kv = strtok(NULL, ";")) {
		/// Get only the filename (key) portion of the key-value pair
		int i = 0;
		while(kv[i] != ':')
			i++;
		kv[i] = '\0';

		if(fs_open(kv) < 0)
			return count;

		count++;
	}

	return count;
}