#include "solution.h"


#include <ext2fs/ext2fs.h>
#include <unistd.h>
#include <errno.h>



int block_transfer(int img, int out, long int block_size, long long int* remainfilesize, int upper_bound, uint32_t* blocks){

	char buf[block_size];
	for (int i = 0; i < upper_bound; i++) {
		int size = *remainfilesize > block_size ? block_size : *remainfilesize;
		if(pread(img, buf, size, block_size*blocks[i]) != size){
			return -errno;
		}
		if(write(out, buf, size) != size){
			return -errno;
		}
		*remainfilesize -= block_size;
		if (*remainfilesize <= 0){
			return 0;
		}
	}
	
	return 1;
}



int dump_file(int img, int inode_nr, int out)
{
	struct ext2_super_block esb = {};
	
	struct ext2_super_block trans_check = {};
	
	//if(pread(img, (char*)&esb, sizeof(struct ext2_super_block), SUPERBLOCK_OFFSET) != sizeof(struct ext2_super_block))
	//	return -errno;

	long int block_size = 1024 << trans_check.s_log_block_size;	
	
	int addr_bg_descr = ((esb.s_first_data_block+1)*block_size + sizeof(struct ext2_group_desc)*((inode_nr-1) / esb.s_inodes_per_group));
	
	struct ext2_group_desc group_desc = {};
	
	if(pread(img, (char*)&group_desc, sizeof(struct ext2_group_desc), addr_bg_descr) != sizeof(struct ext2_group_desc))
		return -errno;

	struct ext2_inode inode = {};
	if(pread(img, (char*)&inode, sizeof(struct ext2_inode), group_desc.bg_inode_table*block_size + ((inode_nr-1) % esb.s_inodes_per_group)*esb.s_inode_size) != sizeof(struct ext2_inode))
		return -errno;

	long long remainfilesize = ((long long)inode.i_size_high << 32L) + (long long)inode.i_size;
	long long siz_chck = ((long long)inode.i_size_high << 32L) + (long long)inode.i_size;
	//printf("%Ld\n", remainfilesize);
	//printf("%ld %d\n", sizeof(struct ext2_inode), esb.s_inode_size);
	uint32_t* x1blocks = (uint32_t*)malloc(block_size);
	uint32_t* x2blocks = (uint32_t*)malloc(block_size);

	
	//int res = block_transfer(img, out, block_size, &remainfilesize, EXT2_IND_BLOCK, inode.i_block);
	
//int block_transfer(int img, int out, long int block_size, long long int* remainfilesize, int upper_bound, uint32_t* blocks){
	int upper_bound = EXT2_IND_BLOCK;
	uint32_t* blocks = inode.i_block;
	char buf[block_size];
	for (int i = 0; i < upper_bound; i++) {
		int size = remainfilesize > block_size ? block_size : remainfilesize;
		if(pread(img, buf, size, block_size*blocks[i]) != size){
			return -errno;
		}
		if(write(out, buf, size) != size){
			return -errno;
		}
		remainfilesize -= block_size;
		if (remainfilesize <= 0){
			res =  0;
		}
	}
	
	res =  1;
	
	
	
	if(res <= 0)
	{
		free(x1blocks);
free(x2blocks);
return res;
	}


	if(pread(img, (char*)x1blocks, block_size, block_size * inode.i_block[EXT2_IND_BLOCK]) != block_size){
		res = -errno;
			
		{
			free(x1blocks);
free(x2blocks);
return res;}
	}
	//res = block_transfer(img, out, block_size, &remainfilesize, block_size/4, x1blocks);
	
	int upper_bound = EXT2_IND_BLOCK;
	uint32_t* blocks = inode.i_block;
	char buf[block_size];
	for (int i = 0; i < upper_bound; i++) {
		int size = remainfilesize > block_size ? block_size : remainfilesize;
		if(pread(img, buf, size, block_size*blocks[i]) != size){
			return -errno;
		}
		if(write(out, buf, size) != size){
			return -errno;
		}
		remainfilesize -= block_size;
		if (remainfilesize <= 0){
			res =  0;
		}
	}
	
	res =  1;
	
	if(res <= 0)
	{
		free(x1blocks);
free(x2blocks);
return res;
	}



	if(pread(img, (char*)x2blocks, block_size, block_size * inode.i_block[EXT2_IND_BLOCK+1]) != block_size){
		res = -errno;
			free(x1blocks);
free(x2blocks);
return res;
	}

	for (int j = 0; j < block_size/4; ++j)
	{
		if(pread(img, (char*)x1blocks, block_size, block_size * x2blocks[j]) != block_size){
			res = -errno;
			free(x1blocks);
free(x2blocks);
return res;
		}
			
		res = block_transfer(img, out, block_size, &remainfilesize, block_size/4, x1blocks);
		if(res <= 0)
		{
			free(x1blocks);
free(x2blocks);
return res;
		}
	}

free(x1blocks);
free(x2blocks);
return res;
}
