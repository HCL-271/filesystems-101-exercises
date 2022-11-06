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

int dr(int img, long int block_size, int upper_bound, uint32_t* blocks){

	char buf[block_size];
	

	for (int i = 0; i < upper_bound; i++) {
		if(blocks[i] == 0)
			return 0;

		if(pread(img, buf, block_size, block_size*blocks[i]) != block_size){
			return -errno;
		}
		struct ext2_dir_entry_2* dir_entry = (struct ext2_dir_entry_2*) buf;

		int remainsize = block_size;
		
		while (remainsize > 0){
			char type = dir_entry -> file_type;
			if(type == EXT2_FT_REG_FILE)
				type = 'f';
			else if(type == EXT2_FT_DIR)
				type = 'd';

			char filename[EXT2_NAME_LEN + 1];
			memcpy(filename, dir_entry -> name, dir_entry -> name_len);
			filename[dir_entry -> name_len] = '\0';

			report_file(dir_entry -> inode, type, filename);

			remainsize -= dir_entry -> rec_len;
			dir_entry = (struct ext2_dir_entry_2*) ((char*) (dir_entry) +  dir_entry -> rec_len);
		}
		
		
	}
	
	return 1;
}

	
	
	


int dump_dir(int img, int inode_nr)
{
	struct ext2_super_block esb = {};
	
	struct ext2_super_block trans_check = {};
	long int lenght = 1024 << esb.s_log_block_size;
	if(pread(img, (char*)&esb, sizeof(struct ext2_super_block), SUPERBLOCK_OFFSET) != sizeof(struct ext2_super_block))
		return -errno;
	int l1 = lenght/4;
	if(pread(img, (char*)&trans_check, sizeof(struct ext2_super_block), SUPERBLOCK_OFFSET) != sizeof(struct ext2_super_block))
		return -errno;

		
	
	int un_adress = ((esb.s_first_data_block+1)*lenght + sizeof(struct ext2_group_desc)*((inode_nr-1) / esb.s_inodes_per_group));
	int freer = 0;
	struct ext2_group_desc gb = {};
	
	if(pread(img, (char*)&gb, sizeof(struct ext2_group_desc), un_adress) != sizeof(struct ext2_group_desc))
		return -errno;

	struct ext2_inode ext2_inode1 = {};
	if(pread(img, (char*)&ext2_inode1, sizeof(struct ext2_inode), gb.bg_inode_table*lenght + ((inode_nr-1) % esb.s_inodes_per_group)*esb.s_inode_size) != sizeof(struct ext2_inode))
		return -errno;

	
	long long siz_chck = ((long long)ext2_inode1.i_size_high << 32L) + (long long)ext2_inode1.i_size;

	uint32_t* var1 = (uint32_t*)malloc(lenght);
	uint32_t* var2 = (uint32_t*)malloc(lenght);
	int res = -1;
	
	
	

	

	
	
	int upper_bound = EXT2_IND_BLOCK;
	uint32_t* blocks = ext2_inode1.i_block;
	char buf[lenght];

	int i = 0;
	
	for (i = 0; i < upper_bound; i++) {
		if(blocks[i] == 0){
			res = 0;
			break;
		}
		else
		{
			res = 1;
		}
			
		if(pread(img, buf, lenght, lenght*blocks[i]) != lenght){
			return -errno;
		}
		struct ext2_dir_entry_2* de = (struct ext2_dir_entry_2*) buf;

		int currfs = lenght;	
		
		while (currfs > 0){
			char type = de -> file_type;
			
			if(type == EXT2_FT_REG_FILE)
			{
				type = 'f';
			}
			else if(type == EXT2_FT_DIR)
			{
				type = 'd';
			}
			char filename[EXT2_NAME_LEN + 1];
			memcpy(filename, de -> name, de -> name_len);
			filename[de -> name_len] = '\0';
				//int img1 = img;
	//int inode_nr1 = inode_nr;
	//int out1 = out
			report_file(de -> inode, type, filename);
			
			currfs -= de -> rec_len;
			
			
			de = (struct ext2_dir_entry_2*) ((char*) (de) +  de -> rec_len);
		}
		
	
	}
	
	//res = dr(img, lenght, EXT2_IND_BLOCK, ext2_inode1.i_block);
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
	
	
	
	
	
	/*
	upper_bound = lenght/4;
	blocks = var1;
	i = 0;
	char buf1[(lenght)];
	
	while( i < upper_bound) {
		if(blocks[i] == 0){
			res = 0;
			break;
		}
		else
		{
			res = 1;
		}
			
		if(pread(img, buf1, lenght, lenght*blocks[i]) != lenght){
			return -errno;
		}
		struct ext2_dir_entry_2* de = (struct ext2_dir_entry_2*) buf1;

		int currfs = lenght;
		
		while (currfs > 0){
			char type = de -> file_type;
			memcpy(filename2, de -> name, de -> name_len);
			if(type == EXT2_FT_REG_FILE)
			{
				type = 'f';
			}
			else if(type == EXT2_FT_DIR)
			{
				type = 'd';
			}
			char filename[EXT2_NAME_LEN + 1];
			memcpy(filename, de -> name, de -> name_len);
			filename[de -> name_len] = '\0';
				//int img1 = img;
	//int inode_nr1 = inode_nr;
	//int out1 = out
			report_file(de -> inode, type, filename);

			currfs -= de -> rec_len;
			de = (struct ext2_dir_entry_2*) ((char*) (de) +  de -> rec_len);
		}
		
	i++;
	}
	i = 0;
	currfs = lenght;
	
	*/
	
	res = dr(img, lenght, lenght/4, var1);
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
			
	
	/*
	upper_bound = l1;
	blocks = var1;
	i = 0;
	char buf2[(lenght)];

	while( i < upper_bound) {
		if(blocks[i] == 0){
			res = 0;
			break;
		}
		else
		{
			res = 1;
		}
			
		if(pread(img, buf2, lenght, lenght*blocks[i]) != lenght){
			return -errno;
		}
		struct ext2_dir_entry_2* de = (struct ext2_dir_entry_2*) buf2;

		
		int currfs = lenght;
		while (currfs > 0){
			char type = de -> file_type;
			memcpy(filename2, de -> name, de -> name_len);
			if(type == EXT2_FT_REG_FILE)
			{
				type = 'f';
			}
			else if(type == EXT2_FT_DIR)
			{
				type = 'd';
			}
			char filename[EXT2_NAME_LEN + 1];
			memcpy(filename, de -> name, de -> name_len);
			filename[de -> name_len] = '\0';
				//int img1 = img;
	//int inode_nr1 = inode_nr;
	//int out1 = out
			report_file(de -> inode, type, filename);

			currfs -= de -> rec_len;
			de = (struct ext2_dir_entry_2*) ((char*) (de) +  de -> rec_len);
		}
		
	i++;
	}
	*/
		res = dr(img, lenght, lenght/4, var1);
		
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
