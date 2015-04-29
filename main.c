
/*
	Runs basic tests for the file system
*/

#include <stdio.h>
#include "fsys.h"

char* get_path(int argc, char** argv);
void mount(char* fsys_path);

int main(int argc, char** argv) {
	char* fsys_path = get_path(argc, argv); // Path to the filesystem virtual disk

	make_fs(fsys_path);
	mount_fs(fsys_path);
	umount_fs(fsys_path);

	return 0;
}

char* get_path(int argc, char** argv) {
	if(argc <= 1) {
		fprintf(stderr, "Must include path to filesystem virtual disk");
		return 0;
	}

	return argv[1];
}