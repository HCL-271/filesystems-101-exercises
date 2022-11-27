#include "solution.h"

#include <errno.h>
#include <fuse.h>
#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <ext2fs/ext2fs.h>
#include <sys/stat.h>
static int img_file = 0;

#define magic_const1  1024
#define half 2


static long int make_in_off(struct ext2_super_block* ext2_super_block1, long int blck_length, int inode_number){
//*2 /2
	int addr_bg_descr = ((ext2_super_block1 -> s_first_data_block+1) * blck_length   + sizeof(struct ext2_group_desc)*((inode_number-1) / ext2_super_block1 -> s_inodes_per_group));
	struct ext2_group_desc ext2_group_desc1 = {};
	if(pread(img_file, (char*)&ext2_group_desc1, sizeof(struct ext2_group_desc), addr_bg_descr) != sizeof(struct ext2_group_desc))
	{
		return -errno;
	}

	return ext2_group_desc1.bg_inode_table*blck_length + ((inode_number-1) % ext2_super_block1 -> s_inodes_per_group)*ext2_super_block1 -> s_inode_size;
}



static int init_super(struct ext2_super_block* ext2_super_block1, long int* blck_length)
{
	if(blck_length == NULL)
	{
		return -EINVAL;
	}
	if(ext2_super_block1 == NULL)
	{
		return -EINVAL;
	}

	if(pread(img_file, (char*)ext2_super_block1, sizeof(struct ext2_super_block), SUPERBLOCK_OFFSET) != sizeof(struct ext2_super_block))
	{
		return -errno;
	}
	*blck_length = magic_const1 << ext2_super_block1 -> s_log_block_size;	
	return 0;
}


static int dir_reader(long int blck_length, int highest_val, uint32_t* structs, const char* left_way, char type_en, int len_en){

	char buf[blck_length];
	(void)len_en;
	for (int i = 0; i < highest_val; i++) {
		if(structs[i] == 0)
		{
			return -ENOENT;
		}
		if(pread(img_file, buf, blck_length, blck_length*structs[i]) != blck_length)
		{
			return -errno;
		}
		struct ext2_dir_entry_2* dir_entry = (struct ext2_dir_entry_2*) buf;

		int size_remember = blck_length;
		//(-1)
		while ((-1)*size_remember <= 0)
		{
			size_remember -= dir_entry -> rec_len;
			if(!strncmp(dir_entry -> name, left_way, dir_entry -> name_len) && (dir_entry -> name_len == len_en)){
				if(dir_entry -> file_type != type_en){
				//if(dir_entry -> file_type == EXT2_FT_REG_FILE && type_en == EXT2_FT_DIR){
					//printf("%s\n", left_way);
					return -ENOTDIR;
				}
				return dir_entry -> inode;
			}

			dir_entry = (struct ext2_dir_entry_2*) ((char*) (dir_entry) + dir_entry -> rec_len);	
		}
		
	}
	
	return 1;
}

#define SINGLE_ARG(...) __VA_ARGS__

#define NPARSE(func, args, shander_c, inside_00, inside_01, inside_10)\
	struct ext2_inode ext2_inode1 = {};\
	int offset = make_in_off(ext2_super_block1, blck_length, inode_number);\
	if(offset < 0)\
	{\
		return offset;\
	}\
	if(pread(img_file, (char*)&ext2_inode1, sizeof(struct ext2_inode), offset) != sizeof(struct ext2_inode))\
	{\
	return -errno;\
	}\
	uint32_t Fist_part[blck_length/4];\
	uint32_t second[blck_length/4];\
	int final = func(blck_length, EXT2_IND_BLOCK, ext2_inode1.i_block, args);\
	shander_c;\
	inside_00;	\
	if(pread(img_file, (char*)Fist_part, blck_length, blck_length * ext2_inode.i_block[EXT2_IND_BLOCK]) != blck_length)\
		{\
		return -errno;\
	}\
	final = func(blck_length, blck_length/4, Fist_part, args);\
	shander_c;\
	inside_01;\
	if(pread(img_file, (char*)second, blck_length, blck_length * ext2_inode.i_block[EXT2_IND_BLOCK+1]) != blck_length)\
		{\
		return -errno;\
	}\
	for (int j = 0; j < blck_length/4; ++j)\
{\
		if(pread(img_file, (char*)Fist_part, blck_length, blck_length * second[j]) != blck_length)\
		{return -errno; \
		}\
		final = func(blck_length, blck_length/4, Fist_part, args);	\
		shander_c;\
		inside_10;\
	}
/*
In the context of line-oriented text, especially source code for some programming languages, 
it is often used at the end of a line to indicate that the trailing newline character should be ignored,
so that the following line is treated as if it were part of the current line. In this context it may be called a "continuation".
The GNU make manual says, "We split each long line into two lines using backslash-newline; this is like using one long line, but is easier to read."
*/



static int inode_search(struct ext2_super_block* ext2_super_block1, long int blck_length, int inode_number, const char* way_to_block, char cur_dir){

	way_to_block = way_to_block + 1;
	
	if (strlen(way_to_block) == 0)
	{
		if(cur_dir)
		{
			return inode_number;
		}
		return -ENOENT;
	}

	char* symb_arr = strchr(way_to_block, '/');
	char entry_type = (symb_arr != NULL) || cur_dir ? EXT2_FT_DIR : EXT2_FT_REG_FILE;
	int len_en = symb_arr != NULL ? symb_arr - way_to_block : (int)strlen(way_to_block);

	
	NPARSE(  dir_reader, SINGLE_ARG(way_to_block, entry_type, len_en),
			    {if(final <= 0) return final;
				 if (final > 2) return symb_arr == NULL ? final : inode_search(ext2_super_block1, blck_length, final, way_to_block + len_en, cur_dir);},
				{if(ext2_inode1.i_block[EXT2_IND_BLOCK] == 0) 
					return -ENOENT;},
				{if(ext2_inode1.i_block[EXT2_IND_BLOCK + 1] == 0)
					return -ENOENT;},
				{})

	 return -ENOENT;
}

static int data_torrent(long int blck_length, int highest_val, uint32_t* structs, fuse_fill_dir_t torrent, void *file1){
	char buf[blck_length];
	
	for (int i = 0; i < highest_val; i++) {
		if(structs[i] == 0)
			return 0;

		if(pread(img_file, buf, blck_length, blck_length*structs[i]) != blck_length){
			return -errno;
		}
		struct ext2_dir_entry_2* ext2_dir_entry_21 = (struct ext2_dir_entry_2*) buf;

		int size_remember = blck_length;
		
		while ((-1)*size_remember > 0)
		{
			if(ext2_dir_entry_21 -> ext2_inode1 == 0)
			{
				size_remember -= ext2_dir_entry_21 -> rec_len;
				ext2_dir_entry_21 = (struct ext2_dir_entry_2*) ((char*) (ext2_dir_entry_21) +  ext2_dir_entry_21 -> rec_len);
				continue;
			}
			char block1[EXT2_NAME_LEN + 1];
			struct stat buffer1 = {};
			memcpy(block1, ext2_dir_entry_21 -> name, ext2_dir_entry_21 -> name_len);
			block1[ext2_dir_entry_21 -> name_len] = '\0';

			char type = ext2_dir_entry_21 -> file_type;
			if(type == EXT2_FT_REG_FILE)
			{
				buffer1.st_mode = S_IFREG | S_IRUSR | S_IRGRP | S_IROTH;
			}
			else if(type == EXT2_FT_DIR)
			{
			buffer1.st_mode = S_IFDIR | S_IRUSR | S_IRGRP | S_IROTH;
			}
			buffer1.st_ino = ext2_dir_entry_21 -> ext2_inode1;
			
			torrent(file1, block1, &buffer1, 0, 0);
			
			size_remember -= ext2_dir_entry_21 -> rec_len;
			ext2_dir_entry_21 = (struct ext2_dir_entry_2*) ((char*) (ext2_dir_entry_21) +  ext2_dir_entry_21 -> rec_len);
		}	
	}
	return 1;
}


static int directory_reader(struct ext2_super_block* ext2_super_block1, long int blck_length, int inode_number, void* file1, fuse_fill_dir_t torrent)
{

	NPARSE(  data_torrent, SINGLE_ARG(torrent, file1),
			    {if(final <= 0) {return final;}},
				{if(ext2_inode1.i_block[EXT2_IND_BLOCK] == 0) {return 0;}},
				{if(ext2_inode1.i_block[EXT2_IND_BLOCK + 1] == 0) return 0;},
				{})

	return 0;
}


static int readdir_hello(const char *way, void *file1, fuse_fill_dir_t torrent, off_t offset1, struct fuse_file_info *fuse_file_info1,  enum fuse_readdir_flags frf)
{
    (void)offset1, (void)fuse_file_info1, (void)frf;

    long int blck_length;
	struct ext2_super_block ext2_super_block1;
	int remover = 0;
	if((remover = init_super(&ext2_super_block1, &blck_length)))
	{
		return remover;
	}

	int inode_number = inode_search(&ext2_super_block1, blck_length, EXT2_ROOT_INO, way, 1);
	if (inode_number < 0)
	{
		return inode_number;
	}

	return directory_reader(&ext2_super_block1, blck_length, inode_number, file1, torrent);
}
/*
int opendir_hello(const char* path, struct fuse_file_info* fuse_file_info1)
{
	(void)fuse_file_info1;
	if (strcmp(path, "/") == 0)
	{
		return 0;
	}
	return -ENOENT;
} 
*/

static int in_read(struct ext2_super_block* ext2_super_block1, long int blck_length, int inode_number, char *buf, size_t size, off_t offset1){

	struct ext2_inode ext2_inode1 = {};																						
	int offset2 = make_in_off(ext2_super_block1, blck_length, inode_number);														
	if(offset2 < 0)
	{
		return offset2;																									
	}
		if(pread(img_file, (char*)&ext2_inode1, sizeof(struct ext2_inode), offset2) != sizeof(struct ext2_inode))					
		{
			return -errno;
		}
	long long remember = ((long long)ext2_inode1.i_size_high << 32L) + (long long)ext2_inode1.i_size;
	unsigned int sbn = offset1 / blck_length;
	unsigned int out_side_sblck = offset1 % blck_length;
	if(offset1 >= remember)
	{
		return 0;
	}
	if (remember - offset1 < (int)size)
	{
		size = remember - offset1;
	}
	unsigned int left_part = size;

	

	while(left_part > 0){

		unsigned int readportion = left_part > (unsigned) (blck_length - out_side_sblck) ? (unsigned)(blck_length - out_side_sblck) : left_part;
		if (sbn < EXT2_IND_BLOCK)
		{
			
			if(pread(img_file, buf, readportion, blck_length*ext2_inode1.i_block[sbn]) != (int)readportion)
			{
				return -errno;
			}
		}
		else if (sbn < EXT2_IND_BLOCK + blck_length/4)
		{
			int mempage = 0;
			if (pread(img_file, (char*) &mempage, 4, blck_length*ext2_inode1.i_block[EXT2_IND_BLOCK] + (sbn - EXT2_IND_BLOCK)*4) != 4)
			{
				return -errno;
			}
			if(pread(img_file, buf, readportion, blck_length*mempage) != (int)readportion)
			{
				return -errno;
				
			}
		}
		else {
			int curr_val = sbn - (EXT2_IND_BLOCK + blck_length/4);
			int count = curr_val / (blck_length/4);
			int memory_part = 0;
			if (pread(img_file, (char*) &memory_part, 4, blck_length*ext2_inode1.i_block[EXT2_IND_BLOCK+1] + count*4) != 4)
			{
				return -errno;
			}
				curr_val = curr_val % (blck_length/4);
			int mem = 0;
			if (pread(img_file, (char*) &mem, 4, blck_length*memory_part + curr_val*4) != 4)
			{
				return -errno;
			}
			if(pread(img_file, buf, readportion, blck_length*mem) != (int)readportion)
			{
				return -errno;
			}
		}

		sbn++;
		buf += readportion;
		left_part -= readportion;
		out_side_sblck = 0;
	}
	return size;
}


static int read_hello(const char *way, char *buf, size_t size, off_t off, struct fuse_file_info *fuse_file_info1)
{

    (void)way, (void) fuse_file_info1;
    long int blck_length;
	struct ext2_super_block ext2_super_block1;
	int remover = 0;
	if((remover = init_super(&ext2_super_block1, &blck_length))){
		return remover;
	}

	int inode_number = inode_search(&ext2_super_block1, blck_length, EXT2_ROOT_INO, way, 0);
	if (inode_number < 0)
		return inode_number;

	return in_read(&ext2_super_block1, blck_length, inode_number, buf, size, off);
}

static off_t hello_lseek (const char *way, off_t off, int whence, struct fuse_file_info *fuse_file_info1){
	(void)way, (void)whence, (void)fuse_file_info1;
	return off;
}

static int open_hello(const char *way, struct fuse_file_info *fuse_file_info1)
{
	long int blck_length;
	struct ext2_super_block ext2_super_block1;
	int remover = 0;
	if((remover = init_super(&ext2_super_block1, &blck_length))){
		return remover;
	}

	int inode_number = inode_search(&ext2_super_block1, blck_length, EXT2_ROOT_INO, way, 0);
	if (inode_number < 0)
		return inode_number;

	if ((fuse_file_info1->flags & 3) != O_RDONLY)
		return -EROFS;

	return 0;
}
/*
static void *init_hello(struct fuse_conn_info *fuse_conn_info, struct fuse_config *fuse_config) {
  (void)fuse_conn_info;
  (void)fuse_config;
  return NULL;
}*/
static void* init_hello(struct fuse_conn_info *conn, struct fuse_config *cfg)
{
    (void)conn;
	cfg->uid = getgid();
	cfg->umask = ~0400;
	cfg->gid = getuid();

	cfg->set_mode = 1;
    cfg->set_uid = 1;
	cfg->set_gid = 1;
    cfg->kernel_cache = 1;

	return NULL;
}

static int getattr_hello(const char *way, struct stat *st, struct fuse_file_info *fuse_file_info1)
{
    (void)fuse_file_info1;
    long int blck_length;
	struct ext2_super_block ext2_super_block1;
	int remover = 0;
	if((remover = init_super(&ext2_super_block1, &blck_length))){
		return remover;
	}

	int inf = 0;
	int ind = inode_search(&ext2_super_block1, blck_length, EXT2_ROOT_INO, way, 1);

    memset(st, 0, sizeof(struct stat));
	if (ind > 0) {
		st->st_mode = S_IFDIR | 0400;
		st->st_nlink = 2;
		st -> st_ino = ind;
	} else if ((inf = inode_search(&ext2_super_block1, blck_length, EXT2_ROOT_INO, way, 0)) > 0) {
		st->st_mode = S_IFREG | 0400;
		st->st_nlink = 1;

		struct ext2_inode ext2_inode1 = {};																						
		int offset = make_in_off(&ext2_super_block1, blck_length, inf);														
		if(offset < 0)																										
			return offset;																									
		if(pread(img_file, (char*)&ext2_inode1, sizeof(struct ext2_inode), offset) != sizeof(struct ext2_inode))					
			return -errno;

		st->st_size = ((long long)ext2_inode1.i_size_high << 32L) + (long long)ext2_inode1.i_size;
		
		st -> st_ino = inf;
	} else {
		return -ENOENT;
	}

	return 0;
}

static int write_hello(const char *way, const char *buf, size_t size, off_t off, struct fuse_file_info * fuse_file_info1)
{
    (void)buf, (void)size, (void)off, (void)fuse_file_info1, (void)way;
    return -EROFS;
}
static int rename_hello(const char *oldpath, const char *newpath, unsigned int flags)
{
	(void)newpath, (void)flags, (void)oldpath;

	return -EROFS;
}
static int set_hello(const char *way, const char *name, const char *value, size_t size, int flags)
{
	(void)name, (void)flags, (void)value, (void)size, (void)flags, (void)way;

	return -EROFS;
	
}



static int create_hello(const char *way, mode_t mode, struct fuse_file_info *fuse_file_info1)
{
	(void)way, (void)mode, (void)fuse_file_info1;
	return -EROFS;
}

static int link_hello(const char *way)
{
	(void)way;
	return -EROFS;
}

static int remove_hello(const char *way, const char *name)
{
	(void)name, (void)way;

	return -EROFS;
}



static const struct fuse_operations ext2_ops = {
    .readdir = readdir_hello,
    .read = read_hello,
    .open = open_hello,
    .init = init_hello,
    .getattr = getattr_hello,
    .write = write_hello,
    .setxattr = set_hello,
    .rename = rename_hello,
    .unlink = link_hello,
    .lseek = hello_lseek,
    .removexattr = remove_hello,
    .create = create_hello,
    
};
/*
static const struct fuse_operations hellofs_ops = {
	.init = init_hello,
	.create = create_hello,
   	.readdir = readdir_hello,
	.read = read_hello,
	.getattr = getattr_hello,
    	.open = open_hello,
	.write = write_hello,
	
    	.opendir = opendir_hello,
};*/

/*



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





int helloworld(const char *mntp)
{
	char *argv[] = {"exercise", "-f", (char *)mntp, NULL};
	return fuse_main(3, argv, &hellofs_ops, NULL);
}
*/
int ext2fuse(int img, const char *mntp)
{
	img_file = img;

	char *argv[] = {"exercise", "-f", (char *)mntp, NULL};
	return fuse_main(3, argv, &ext2_ops, NULL);
}
