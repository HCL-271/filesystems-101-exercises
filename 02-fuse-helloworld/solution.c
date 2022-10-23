#include "solution.h"
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>	
#include <stddef.h>
#include <stdio.h>
#include <fuse.h>
const char *path1 = "hello";

static void *init_hello(struct fuse_conn_info *fuse_conn_info, struct fuse_config *fuse_config) {
  (void)fuse_conn_info;
  (void)fuse_config;
  return NULL;
}


static int create_hello(const char* path, mode_t mode, struct fuse_file_info* fuse_file_info){
  (void)path;
  (void)mode;
  (void)fuse_file_info;
  return -EROFS;

}

static int readdir_hello(const char *path, void *buffer, fuse_fill_dir_t useless_variable_here, off_t offsett, struct fuse_file_info *fuse_file_info, enum fuse_readdir_flags fuse_readdir_flags1)
{
	(void)offsett;
	(void)fuse_file_info;
	(void)fuse_readdir_flags1;
	
	if (strcmp(path, "/") != 0)
	{
		return -ENOENT;
	}
	
	fill(buffer, ".", NULL, 0, 0);
	fill(buffer, "..", NULL, 0, 0);
	fill(buffer, path1, NULL, 0, 0);
	return 0;
}


static const struct fuse_operations hellofs_ops = {
	.init = init_hello
	.create = create_hello,
   	.readdir = readdir_hello,
};

int helloworld(const char *mntp)
{
	char *argv[] = {"exercise", "-f", (char *)mntp, NULL};
	return fuse_main(3, argv, &hellofs_ops, NULL);
}

