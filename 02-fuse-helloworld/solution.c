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

static int readdir_hello(const char *path, void *buffer, fuse_fill_dir_t fuse_fill_dir_t1, off_t offsett, struct fuse_file_info *fuse_file_info, enum fuse_readdir_flags fuse_readdir_flags1)
{
	(void)offsett;
	(void)fuse_file_info;
	(void)fuse_readdir_flags1;
	
	if (strcmp(path, "/") != 0)
	{
		return -ENOENT;
	}
	
	fuse_fill_dir_t1(buffer, ".", NULL, 0, 0);
	fuse_fill_dir_t1(buffer, "..", NULL, 0, 0);
	fuse_fill_dir_t1(buffer, path1, NULL, 0, 0);
	return 0;
}

static int read_hello(const char *path, char *bufer, size_t size, off_t offsett, struct fuse_file_info *fuse_file_info1)
{
	if (strcmp(path+1, path1) != 0)
	{
		return -ENOENT;
	}
	
	struct fuse_context* fuse_context1 = fuse_get_context();
	pid_t is_pid = fuse_context1->pid;
	
        size_t width = snprintf(NULL, 0, "hello, %d\n", is_pid);
        (void) fuse_file_info1;
	
	char* file = (char*) malloc (width);
	sprintf(file, "hello, %d\n", is_pid);
	
        if (offsett < (long int)width) {
                if (offsett + size > width)
		{
                        size = width - offsett;
		}
		memcpy(bufer, file + offsett, size);
        } else {
                size = 0;
	}

	free(file);
        return (width - offsett);
}
static int getattr_hello(const char *path, struct stat *stat,
                           struct fuse_file_info *fuse_file_info1) {
  (void)fuse_file_info1;
	
  memset(stat, 0, sizeof(struct stat));
  if (strcmp(path1, "/") == 0)
  {
    stat->st_mode = S_IFDIR | S_IRUSR;
    stat->st_nlink = 2;
	return 0;
	  
  } else if (strcmp(path+1, path1) == 0) 
  {
    stat->st_mode = S_IFREG | S_IRUSR;
    stat->st_nlink = 1;
    stat->st_size = 496;
	return 0;
  }

  return -ENOENT;
}

static int open_hello(const char *path, struct fuse_file_info *fuse_file_info1) {
  if (strcmp(path+1, path1) != 0)
  {
	  return -ENOENT;
  }
	
  if ((fuse_file_info1->flags & O_ACCMODE) != O_RDONLY) 
  {
	  return -EROFS;
  }
  return 0;
}	    
	    
static const struct fuse_operations hellofs_ops = {
	.init = init_hello,
	.create = create_hello,
   	.readdir = readdir_hello,
	.read = read_hello,
	.getattr = getattr_hello,
    	.open = open_hello
    
};

int helloworld(const char *mntp)
{
	char *argv[] = {"exercise", "-f", (char *)mntp, NULL};
	return fuse_main(3, argv, &hellofs_ops, NULL);
}

