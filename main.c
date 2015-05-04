
/*
	Runs basic tests for the file system
*/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "fsys.h"
#include "disk.h"

#define SIZE_DUMP 8200

char* get_path(int argc, char** argv);
void mount(char* fsys_path);
int spam_files();
int open_files();
void string_gen(char* buf, int nbytes);

int main(int argc, char** argv) {
	char dump[SIZE_DUMP];
	char* fsys_path = get_path(argc, argv); // Path to the filesystem virtual disk

	srand(time(NULL));

	printf("--- Making initial file system ---\n");
	make_fs(fsys_path);
	printf("--- File system created --- \n\n");
	printf("--- Mounting file system ---\n");
	mount_fs(fsys_path);
	printf("--- File system mounted --- \n\n");

	printf("--- Writing to files ---\n");
	fs_create("foo");
	fs_create("tester");
	int file = fs_open("foo");
	int file2 = fs_open("tester");
	string_gen(dump, SIZE_DUMP);
	fs_write(file, dump, SIZE_DUMP);
	fs_write(file2, dump, SIZE_DUMP);
	printf("--- Wrote to files ---\n\n");

	printf("--- Spamming test files ---\n");
	printf("Created %d new files\n", spam_files());
	printf("--- New files created --- \n\n");

	printf("--- Opening files ---\n");
	printf("Opened %d files before filesystem rejected request\n", open_files());
	printf((fs_open("foo") > 0) ? "Could open a file a second time - bad" : "Couldn't open a file a second time - good\n");
	printf("--- Opened files ---\n\n");

	printf("--- Closing some files ---\n");
	fs_close(file2);
	if(fs_write(file2, dump, SIZE_DUMP) < 0) {
		printf("Writing to a closed file failed. This is good news\n");
	} else {
		printf("Writing to a closed file succeeded. This is bad news\n");
	}
	printf("--- Files closed ---\n\n");

	printf("--- Reopening closed file ---\n");
	file2 = fs_open("tester");
	char tmp_buff[25] = "***1********************";
	fs_write(file2, tmp_buff, 2400);
	print_block(0);
	print_block(1);
	print_block(2);
	print_block(3);
	print_block(4);
	print_block(5);
	print_block(6);
	printf("--- Reopened closed file ---\n\n");
	
	printf("--- Testing Reading Files ---\n");
	char buf[25];
	fs_close(file2);
	file2 = fs_open("tester");

	fs_read(file2, buf, 24);
	printf("Reading 24 bytes from start of file: %s\n", buf);
	printf("--- Files read ---\n\n");
	
	printf("--- Seeking File ---\n");
	char buf2[7];
	strcpy(buf2, "-------");

	fs_close(file2);
	file2 = fs_open("tester");

	printf("lseek errcode == %d\n", fs_lseek(file2, 4096));
	fs_write(file2, buf2, 7);
	print_block(2);
	printf("--- Finished seeking ---\n\n");
	
	printf("--- Checking file size ---\n");
	printf("Size of tester file: %d\n", fs_get_filesize(file2));
	printf("--- File size check complete---\n\n");

	printf("--- Unmounting filesystem ---\n");
	umount_fs(fsys_path);
	printf("--- Filesystem unmounted ---\n\n");

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
	int err_code = 0;

	do {
		count++;
		sprintf(fname, "t%d", count);
		err_code = fs_create(fname);
	} while(err_code == 0);

	printf("Filesystem declined new file with error code %d\n", err_code);

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

		int ret_val = fs_open(kv);
		if(ret_val < 0) {
			return count;
		}

		count++;
	}

	return count;
}

/**
 * Generates a random string of given length
 */
void string_gen(char* buf, int nbytes) {
	int i = 0;
	for(; i <= nbytes - 1; i++) {
		buf[i] = '1';//(rand() % 95) + 32; // Random printable character
		//(rand() % 95) + 32; // Random printable character
	}
	buf[nbytes] = '\0';
}