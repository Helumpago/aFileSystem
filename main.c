
/*
	Runs basic tests for Team RedPizzaBaron's file system
*/

#include <stdio.h>

int main(int argc, char** argv) {
	char* fsys_path = NULL; // Path to the filesystem virtual disk

	/// Get command line arguments
	if(argc <= 1) {
		fprintf(stderr, "Must include path to filesystem virtual disk");
		return 0;
	}

	fsys_path = argv[1];
	printf("Virtual disk is %s\n", fsys_path);

	/// Create new filesystem
	make_fs(fsys_path);

	return 0;
}