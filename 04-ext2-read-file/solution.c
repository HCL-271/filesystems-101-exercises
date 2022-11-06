#include "solution.h"
#include <ext2fs/ext2fs.h>
#include <unistd.h>
#include <errno.h>


int make_free(uint32_t* a,uint32_t* b)
{
	free(a);
	free(b);
	return 0;
}

int dump_file(int img, int inode_nr, int out)
{
	struct ext2_super_block esb = {};
	
	struct ext2_super_block trans_check = {};
	long int lenght = 1024 << esb.s_log_block_size;
	if(pread(img, (char*)&esb, sizeof(struct ext2_super_block), SUPERBLOCK_OFFSET) != sizeof(struct ext2_super_block))
		return -errno;
	
	if(pread(img, (char*)&trans_check, sizeof(struct ext2_super_block), SUPERBLOCK_OFFSET) != sizeof(struct ext2_super_block))
		return -errno;
	int i = 0;
	//long int block_size = 1024 << trans_check.s_log_block_size;	
	
	int un_adress = ((esb.s_first_data_block+1)*lenght + sizeof(struct ext2_group_desc)*((inode_nr-1) / esb.s_inodes_per_group));
	int freer = 0;
	struct ext2_group_desc gb = {};
	
	if(pread(img, (char*)&gb, sizeof(struct ext2_group_desc), un_adress) != sizeof(struct ext2_group_desc))
		return -errno;

	struct ext2_inode ext2_inode1 = {};
	if(pread(img, (char*)&ext2_inode1, sizeof(struct ext2_inode), gb.bg_inode_table*lenght + ((inode_nr-1) % esb.s_inodes_per_group)*esb.s_inode_size) != sizeof(struct ext2_inode))
		return -errno;

	long long currfs = ((long long)ext2_inode1.i_size_high << 32L) + (long long)ext2_inode1.i_size;
	long long siz_chck = ((long long)ext2_inode1.i_size_high << 32L) + (long long)ext2_inode1.i_size;

	uint32_t* var1 = (uint32_t*)malloc(lenght);
	uint32_t* var2 = (uint32_t*)malloc(lenght);
	int res = -1;
	
	int upper_bound = EXT2_IND_BLOCK;
	uint32_t* blocks = ext2_inode1.i_block;
	char buf[lenght];
	for (int i = 0; i < upper_bound; i++) {
		int size = currfs > lenght ? lenght : currfs;
		if(pread(img, buf, size, lenght*blocks[i]) != size){
			return -errno;
		}
		if(write(out, buf, size) != size){
			return -errno;
		}
		currfs -=lenght;
		if (currfs <= 0){
			res =  0;
		}
	}
	
	res =  1;
	
	
	
	if(res <= 0)
	{
		free(var1);
		free(var2);
		return res;
	}

	siz_chck += 1;
	if(pread(img, (char*)var1, lenght, lenght * ext2_inode1.i_block[EXT2_IND_BLOCK]) != lenght){
		res = -errno;
			
		{
			free(var1);
			free(var2);
			return res;
		}
	}
	
	
	upper_bound = lenght/4;
	blocks = var1;
	int l1 = lenght/4;
	char buf1[(lenght/4)];
	while ( i < upper_bound) {
		int size = currfs > l1 ? l1 : currfs;
		if(pread(img, buf1, size,l1*blocks[i]) != size){
			return -errno;
		}
		i++;
		
	}
	i = 0;
	while ( i < upper_bound) {
		int size = currfs > l1 ?l1 : currfs;
		if(write(out, buf, size) != size){
			return -errno;
		}
		currfs -= l1;
		if (currfs <= 0){
			res =  0;
		}
		i++;
	}
	res =  1;
	
	if(res <= 0)
	{
		
		free(var1);
			free(var2);
		return res;
	}



	if(pread(img, (char*)var2, lenght, lenght * ext2_inode1.i_block[EXT2_IND_BLOCK+1]) !=lenght){
		res = -errno;
		free(var1);
			free(var2);
		return res;
	}

	for (int j = 0; j <l1; ++j)
	{
		if(pread(img, (char*)var1, lenght,lenght * var2[j]) != lenght){
			res = -errno;
			free(var1);
			free(var2);
			return res;
		}
			
		upper_bound = l1;
	blocks = var1;
	
	char buf2[l1];
	i = 0;
	while ( i < upper_bound) {
		int size = currfs >l1 ? l1 : currfs;
		if(pread(img, buf2, size, l1*blocks[i]) != size){
			return -errno;
		}
		i++;
		
	}
	i = 0;
	while ( i < upper_bound) {
		int size = currfs > lenght/4 ? lenght/4 : currfs;
		if(write(out, buf, size) != size){
			return -errno;
		}
		currfs -= l1;
		if (currfs <= 0){
			res =  0;
		}
		i++;
	}
	
	res =  1;
		
		if(res <= 0)
		{
		free(var1);
			free(var2);
		return res;
		}
	}
	freer += 1;
	if (freer < 0){
		return errno;
	}

		free(var1);
			free(var2);
return res;
}
