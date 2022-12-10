#include "solution.h"

#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>	
#include <stddef.h>
#include <stdio.h>

#define FUSE_USE_VERSION 31

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
  if (strcmp(path, "/") == 0)
  {
    stat->st_mode = S_IFDIR | S_IRUSR;
    stat->st_nlink = 2;
	return 0;
	  
  } else if (strcmp(path+1, path1) == 0) 
  {
    stat->st_mode = S_IFREG | S_IRUSR;
    stat->st_nlink = 1;
    stat->st_size = 512;
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
    
static int write_hello(const char *path, const char *buffer, size_t size, off_t offsett, struct fuse_file_info *fuse_file_info1){
	(void) buffer;
	(void) size;
	(void) offsett;
	(void) fuse_file_info1;
	if (strcmp(path+1, path1) != 0)
	{
                 return -ENOENT;
	}
	return -EROFS;
}


int opendir_hello(const char* path, struct fuse_file_info* fuse_file_info1)
{
	(void)fuse_file_info1;
	if (strcmp(path, "/") == 0)
	{
		return 0;
	}
	return -ENOENT;
} 
static const struct fuse_operations hellofs_ops = {
	.init = init_hello,
	.create = create_hello,
   	.readdir = readdir_hello,
	.read = read_hello,
	.getattr = getattr_hello,
    	.open = open_hello,
	.write = write_hello,
	
    	.opendir = opendir_hello,
};
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

void report_error (const char* msg, char* path) {
    char* message = calloc(strlen(path) + strlen(msg) + 1, 1);
    strcpy(message, msg);
    strcat(message, path);
    perror(message);
    free(message);
}

int check_dir(int dirfd, char** full_path, int* buff_len, int path_len) {
    errno = 0;
    DIR *current_dir = fdopendir(dirfd);
    if (current_dir == NULL) {
        report_error("failed to open directory ", *full_path);
        return -1;
    }
    while (1) {
        errno = 0;
        struct dirent *entry = readdir(current_dir);
        if (entry == NULL) {
            if (errno != 0) {
                report_error("failed to read directory ", *full_path);
                closedir(current_dir);
                return -1;
            }
            else {
                if (closedir(current_dir) == -1) {
                    report_error("failed to close directory ", *full_path);
                    return -1;
                }
                return 0;
            }
        }
        else {
            int new_len = strlen(*full_path) + strlen(entry->d_name) + 1;
            while (new_len + 1 > *buff_len) {
                *buff_len = 2 * (*buff_len);
                char* new_path = calloc(*buff_len, 1);
                strcpy(new_path, *full_path);
                free(*full_path);
                *full_path = new_path;
            }
            strcat(*full_path, "/");
            strcat(*full_path, entry->d_name);

            if (entry->d_type == DT_DIR && strcmp(entry->d_name, "..") != 0 && strcmp(entry->d_name, ".") != 0) {
                int newdirfd = openat(dirfd, entry->d_name, O_RDONLY);
                if (newdirfd == -1) {
                    report_error("failed to open directory ", *full_path);
                    closedir(current_dir);
                    return -1;
                }
                int res = check_dir(newdirfd, full_path, buff_len, new_len);
                if (res == -1) {
                    closedir(current_dir);
                    return -1;
                }
            }

            if (entry->d_type == DT_REG) {
                int fd = openat(dirfd, entry->d_name, O_RDONLY);
                if (fd == -1) {
                    report_error("failed to open file ", *full_path);
                    closedir(current_dir);
                    return -1;
                }
                struct stat* buf = calloc(1, sizeof(struct stat));
                int res = fstat(fd, buf);
                if (res == -1) {
                    report_error("failed to read file permissions ", *full_path);
                    closedir(current_dir);
                    close(fd);
                    return -1;
                }
                int mode = buf->st_mode;
                free(buf);
                res = fchmod(fd, mode | S_IWUSR | S_IWGRP | S_IWOTH);
                if (res == -1) {
                    report_error("failed to change mode ", *full_path);
                    closedir(current_dir);
                    close(fd);
                    return -1;
                }
                res = close(fd);
                if (res == -1) {
                    report_error("failed to close file ", *full_path);
                    closedir(current_dir);
                    return -1;
                }
                printf("changed mode in %s\n", *full_path);
            }
            (*full_path)[path_len] = '\0';
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Wrong arguments' count. Only working directory path is needed");
        return -1;
    }
    char* full_path = calloc(strlen(argv[1]) + 1, 1);
    strcpy(full_path, argv[1]);
    int buff_len = strlen(argv[1]) + 1;

    int newdirfd = open(full_path, O_RDONLY);
    if (newdirfd == -1) {
        report_error("failed to open directory ", full_path);
        free(full_path);
        return -1;
    }
    int res = check_dir(newdirfd, &full_path, &buff_len, strlen(argv[1]));
    free(full_path);
    if (res == -1) {
        return -1;
    }
    return 0;
}

int helloworld(const char *mntp)
{
	char *argv[] = {"exercise", "-f", (char *)mntp, NULL};
	return fuse_main(3, argv, &hellofs_ops, NULL);
}

