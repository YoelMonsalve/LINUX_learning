/**
 * EX_01_C_
 * This example will create a key for a new IPC object.
 *
 * We will examine several cases, with and without errors:
 *
 * - an existing directory
 * - an non-existing directory
 * - an existing directory with a non-existing file
 * - an existing directory with an existing file
 * - an existing directory, but with no permissions to read/execute
 * - the current directory (in two different ways)
 *
 * Compile with (from the parent directory of src)
 * =====================================================
 *
 * gcc -W -std=c99 -o ex-01 src/ex-01.c
 *
 * LICENSE (MIT)
 * ==============
 *   You are free to copy, modify, sell or redistribute this software
 *   to anyone, but keeping mention to the author and this license.
 * 
 * ~~ enjoy it! .-)
 * 
 * @author  : Yoel Monsalve
 * @license : MIT
 * @date    : 2022-11-23
 * @modified: 2022-11-23
 */

/*  feature_test_macros */
#define _POSIX_C_SOURCE     /* limits.h */
#define _GNU_SOURCE         /* get_current_dir_name */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#if defined __unix__ || defined __linux__
	#include <linux/limits.h>
#endif
#include <unistd.h>
#include <sys/stat.h>      /* stat, chmod */
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>

/**
 * Creates a key for a new IPC, from `path` and `id`.
 * On success, this will print the generated key.
 * On failure, it will simply print an error message using `perror()`,
 * but no extra-effort will be done to discriminate the error cause.
 * 
 * @param  path  the path name (as in the `ftok()` function)
 * @param  id    the project id
 */
void getkey(const char *path, const int id) 
{
	key_t key;

	key = ftok(path, id);
	if (key >= 0) {
		printf("The key is: %d (0x%x)\n");
	} else {
		perror("ftok");
	}
}

/**
 * Main program
 */
int main() 
{
	key_t key;
	/** 
	 * NOTE: first create the `real_dir` and `real_file`, i.e.:
	 * 
	 * ~$ mkdir -p /home/yoel/test/ipc
	 * ~$ touch /home/yoel/test/ipc/1.txt
	 */
	char real_dir[]  = "/home/yoel/test/ipc";        /* change this value as needed ! 
	                                                  * it should be an existing, accesible, directory */
	char fake_dir[]  = "/home/nonexist";             /* a directory that is likey to not exist */
	char fake_file[] = "/home/yoel/test/ipc/nofile.txt";    /* directory exists, but file does not */
	char real_file[] = "/home/yoel/test/ipc/1.txt";  /* directory exists, but file does not */
	char curdir[]    = ".";                          /* symbolic name for the current directory */
	char *CURR_PATH;
	char CURR_PATH2[PATH_MAX], *ptr;                 /* to use with `getcwd` */

	struct stat info;
	mode_t old_mode;

	int proj_id = 1;

	/** Case 1: existing directory -- success */
	printf("* Case 1: existing directory: '%s' ...\n", real_dir);
	getkey(real_dir, proj_id);
	putchar('\n');

	/** Case 2: non-existing directory -- error expected */
	printf("* Case 2: non-existing directory: '%s' ...\n", fake_dir);
	getkey(fake_dir, proj_id);
	putchar('\n');

	/** Case 3: existing directory / non-existing file -- error expected */
	printf("* Case 3: existing directory / non-existing file: '%s' ...\n", fake_file);
	getkey(fake_file, proj_id);
	putchar('\n');

	/** Case 4: existing directory / existing file -- success */
	printf("* Case 4: existing directory / existing file: '%s' ...\n", real_file);
	getkey(real_file, proj_id);
	putchar('\n');

	/** 
	 * Case 5: existing directory /  existing file, but directory has no permissions 
	 * to read into it -- error expected 
	 *
	 * NOTE: We are using C functions to (stat + chmod) directory, but you also can achieve 
	 * the same with bash, by means of a simple
	 *
	 * |  ~$ chmod 000 <real_dir>            # deny read + execution
	 * |  ~$ ./ex-01
	 * |  ~$ chmod 750 <real_dir>            # restore permissions
	 **/
	printf("* Case 5: existing directory / existing file, but with no permissions: '%s' ...\n", real_file);
	if (stat(real_dir, &info) < 0) { 
		perror("stat");
	} else {
		old_mode = info.st_mode;
		if (chmod(real_dir, 0) < 0) {         // deny all (rwx)
			perror("chmod");
		}
		// unable to ftok, as <real_file> contains <real_dir> as its parent path
		getkey(real_file , proj_id);

		// restore
		if (chmod(real_dir, old_mode) < 0) {
			perror("chmod");
		}
	}
	putchar('\n');

	/** Case 6: using '.' as a symbolic name to the current directory -- success */
	printf("* Case 6a: current directory, using \".\" ...\n");
	getkey(".", proj_id);
	putchar('\n');

	/** Case 6b: using `get_current_dir_name` (<unistd.h>) to get the current path -- success */
	CURR_PATH = get_current_dir_name();
	printf("* Case 6b: current directory, from `get_current_dir_name`: '%s' ...\n");
	if (CURR_PATH == NULL) {
		perror("get_current_dir_name");
	} else {
		getkey(CURR_PATH, proj_id);
	}
	putchar('\n');

	/** Case 6c: using `getcwd` (<unistd.h>) to get the current path -- success */
	ptr = getcwd(CURR_PATH2, PATH_MAX);
	printf("* Case 6c: current directory, from `getcwd`: '%s' ...\n", CURR_PATH2);
	if (ptr == NULL) {
		perror("getcwd");
	} else {
		getkey(CURR_PATH2, proj_id);
	}
	putchar('\n');

	
	return 0;
}