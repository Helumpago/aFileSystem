
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

	/**
	 * Create the file system. 
	 */
	printf("--- Making initial file system ---\n");
	make_fs(fsys_path);
	printf("--- File system created --- \n\n");

	/**
	 * Mount the file system. 
	 */
	printf("--- Mounting file system ---\n");
	mount_fs(fsys_path);
	printf("--- File system mounted --- \n\n");

	/**
	 * Some data to a couple of files
	 * Creates two files: foo and tester 
	 * Writes 8200 "1"s to each file. This is enough data to require
	 * spreading each file across 3 blocks
	 */
	printf("--- Writing to files ---\n");

	/// Create and open the files
	fs_create("foo");
	fs_create("tester");
	int file = fs_open("foo");
	int file2 = fs_open("tester");
	/// Write to both files
	string_gen(dump, SIZE_DUMP);
	fs_write(file, dump, SIZE_DUMP);
	fs_write(file2, dump, SIZE_DUMP);
	printf("--- Wrote to files ---\n\n");

	/**
	 * Create as many files as possible, each containing no actual data
	 * On our machine, the file system can create 475 files before refusing - 
	 * better than the 64 files required by the assignment.
	 */
	printf("--- Spamming test files ---\n");
	printf("Created %d new files\n", spam_files());
	printf("--- New files created --- \n\n");

	/**
	 * Open as many of the files as possible.
	 * Our tests show the file system can open 33 files at the same time
	 */
	printf("--- Opening files ---\n");
	printf("Opened %d files before filesystem rejected request\n", open_files());

	/**
	 * Try to open a file that is already open. 
	 */
	printf((fs_open("foo") > 0) ? "Could open the same file a second time - bad" : "Couldn't open the same file a second time - good\n");
	printf("--- Opened files ---\n\n");

	/**
	 * Closes one of the files, and then tries writing to the handle to 
	 * ensure that the file closed properly
	 */
	printf("--- Closing some files ---\n");
	fs_close(file2);
	if(fs_write(file2, dump, SIZE_DUMP) < 0) {
		printf("Writing to a closed file failed. This is good news\n");
	} else {
		printf("Writing to a closed file succeeded. This is bad news\n");
	}
	printf("--- Files closed ---\n\n");

	/**
	 * Try reopening the file to ensure that the file descriptor table
	 * was properly cleaned and that files can still be openend
	 *
	 * After reopening, write the string "***1********************"
	 * to the file. Reopening the file reset the cursor, so the string
	 * should be written to the start of the file.
	 */
	printf("--- Reopening closed file ---\n");
	file2 = fs_open("tester");
	char tmp_buff[25] = "***1********************";
	fs_write(file2, tmp_buff, 2400);
	printf("--- Reopened closed file ---\n\n");
	
	/**
	 * Try reading the data that was just written. First, lseek to
	 * the start of the file, and then read in 24 bytes of data.
	 * This is not enough data to span a block.
	 * 
	 * This test shows that fs_lseek() and fs_read() work
	 */
	printf("--- Testing Reading Files ---\n");
	char buf[25];
	fs_lseek(file2, 0);

	printf("Number of bytes read: %d\n", fs_read(file2, buf, 24));
	printf("\tBytes read: %s\n\n", buf);
	
	/**
	 * Read 8500 bytes from the file foo.
	 * This read is large enough to span 3 blocks. It shows that our read
	 * works for large read sizes
	 */
	fs_lseek(file, 0);
	char buff[8500];
	
	printf("Number of bytes read: %d\n", fs_read(file, buff, 8500));
	printf("\tBytes read: %s\n", buff);
	printf("--- Files read ---\n\n");
	
	/**
	 * Get the size of one of our files (tester). This file spans 3 blocks,
	 * which means it should return "12288"
	 * This test shows that the fs_get_filesize works for getting the number
	 * of blocks that a file uses
	 */
	printf("--- Checking file size ---\n");
	printf("Size of \"tester\" file: %d\n", fs_get_filesize(file2));
	printf("--- File size check complete---\n\n");

	/**
	 * Truncate the file down to 4000 bytes and print the new file size.
	 * This truncation will free all but the first block, so the size that
	 * should be prined is 4096.
	 */
	printf("--- Trying to truncate ---\n");
	fs_truncate(file2, 4000);
	printf("Size of tester file: %d\n", fs_get_filesize(file2));
	printf("--- Finished truncation ---\n\n");

	/**
	 * Try deleting the file. After deletion, try to reopen. This fails, as expected.
	 */
	printf("--- Trying to delete ---\n");	
	fs_close(file2);
	fs_delete("tester");
	printf(fs_open("tester") > 0 ? "Could open file after it is deleted - bad\n" : "Couldn't open file after was deleted - good\n");
	printf("--- Finished deleting ---\n\n");

	/**
	 * Create a new file. Recall that prior to deleting the "tester" file, the file system
	 * couldn't support any more files. So, the fact that we are now able to create a new file
	 * shows that the meta section was properly cleaned, allowing new files to be created.
	 */
	printf("--- Trying to create in the new space ---\n");	
	fs_create("aNewFile");
	printf("--- Create finished ---\n\n");	

	/**
	 * Dismount the filesystem
	 */
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
		if(ret_val < 0 && ret_val != ALREADY_OPEN) {
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