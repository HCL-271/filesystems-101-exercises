#include "solution.h"
#include <ext2fs/ext2fs.h>
#include <unistd.h>
#include <errno.h>




int dump_file(int img, int inode_nr, int out)
{
	struct ext2_super_block esb = {};
	
	struct ext2_super_block trans_check = {};
	
	if(pread(img, (char*)&esb, sizeof(struct ext2_super_block), SUPERBLOCK_OFFSET) != sizeof(struct ext2_super_block))
		return -errno;
	
	if(pread(img, (char*)&trans_check, sizeof(struct ext2_super_block), SUPERBLOCK_OFFSET) != sizeof(struct ext2_super_block))
		return -errno;

	//long int block_size = 1024 << trans_check.s_log_block_size;	
	
	int un_adress = ((esb.s_first_data_block+1)*(1024 << trans_check.s_log_block_size) + sizeof(struct ext2_group_desc)*((inode_nr-1) / esb.s_inodes_per_group));
	
	struct ext2_group_desc gb = {};
	
	if(pread(img, (char*)&gb, sizeof(struct ext2_group_desc), un_adress) != sizeof(struct ext2_group_desc))
		return -errno;

	struct ext2_inode ext2_inode1 = {};
	if(pread(img, (char*)&ext2_inode1, sizeof(struct ext2_inode), gb.bg_inode_table*(1024 << trans_check.s_log_block_size) + ((inode_nr-1) % esb.s_inodes_per_group)*esb.s_inode_size) != sizeof(struct ext2_inode))
		return -errno;

	long long currfs = ((long long)ext2_inode1.i_size_high << 32L) + (long long)ext2_inode1.i_size;
	long long siz_chck = ((long long)ext2_inode1.i_size_high << 32L) + (long long)ext2_inode1.i_size;

	uint32_t* x1blocks = (uint32_t*)malloc((1024 << trans_check.s_log_block_size));
	uint32_t* x2blocks = (uint32_t*)malloc((1024 << trans_check.s_log_block_size));
	int res = -1;
	
	int upper_bound = EXT2_IND_BLOCK;
	uint32_t* blocks = ext2_inode1.i_block;
	char buf[(1024 << trans_check.s_log_block_size)];
	for (int i = 0; i < upper_bound; i++) {
		int size = currfs > (1024 << trans_check.s_log_block_size) ? (1024 << trans_check.s_log_block_size) : currfs;
		if(pread(img, buf, size, (1024 << trans_check.s_log_block_size)*blocks[i]) != size){
			return -errno;
		}
		if(write(out, buf, size) != size){
			return -errno;
		}
		currfs -= (1024 << trans_check.s_log_block_size);
		if (currfs <= 0){
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


	if(pread(img, (char*)x1blocks, (1024 << trans_check.s_log_block_size), (1024 << trans_check.s_log_block_size) * ext2_inode1.i_block[EXT2_IND_BLOCK]) != (1024 << trans_check.s_log_block_size)){
		res = -errno;
			
		{
			free(x1blocks);
			free(x2blocks);
			return res;
		}
	}
	
	
	upper_bound = (1024 << trans_check.s_log_block_size)/4;
	blocks = x1blocks;
	
	buf[(1024 << trans_check.s_log_block_size)/4];
	for (int i = 0; i < upper_bound; i++) {
		int size = currfs > (1024 << trans_check.s_log_block_size)/4 ? (1024 << trans_check.s_log_block_size)/4 : currfs;
		if(pread(img, buf, size, (1024 << trans_check.s_log_block_size)/4*blocks[i]) != size){
			return -errno;
		}
		if(write(out, buf, size) != size){
			return -errno;
		}
		currfs -= (1024 << trans_check.s_log_block_size)/4;
		if (currfs <= 0){
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



	if(pread(img, (char*)x2blocks, (1024 << trans_check.s_log_block_size), (1024 << trans_check.s_log_block_size) * ext2_inode1.i_block[EXT2_IND_BLOCK+1]) != (1024 << trans_check.s_log_block_size)){
		res = -errno;
			free(x1blocks);
free(x2blocks);
return res;
	}

	for (int j = 0; j < (1024 << trans_check.s_log_block_size)/4; ++j)
	{
		if(pread(img, (char*)x1blocks, (1024 << trans_check.s_log_block_size), (1024 << trans_check.s_log_block_size) * x2blocks[j]) != (1024 << trans_check.s_log_block_size)){
			res = -errno;
			free(x1blocks);
free(x2blocks);
return res;
		}
			
		upper_bound = (1024 << trans_check.s_log_block_size)/4;
	blocks = x1blocks;
	
	buf[(1024 << trans_check.s_log_block_size)/4];
	for (int i = 0; i < upper_bound; i++) {
		int size = currfs > (1024 << trans_check.s_log_block_size)/4 ? (1024 << trans_check.s_log_block_size)/4 : currfs;
		if(pread(img, buf, size, (1024 << trans_check.s_log_block_size)/4*blocks[i]) != size){
			return -errno;
		}
		if(write(out, buf, size) != size){
			return -errno;
		}
		currfs -= (1024 << trans_check.s_log_block_size)/4;
		if (currfs <= 0){
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
	}

free(x1blocks);
free(x2blocks);
return res;
}
