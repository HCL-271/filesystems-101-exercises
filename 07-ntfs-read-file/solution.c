#include "solution.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <ext2fs/ext2fs.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/uio.h>
#define __timespec_defined

#include <ntfs-3g/types.h>
#include <ntfs-3g/attrib.h>
#include <ntfs-3g/volume.h>
#include <ntfs-3g/dir.h>

ntfs_inode* path_to_node(ntfs_volume *ntfs_volume1, ntfs_inode *ntfs_inode1, const char *way)
{


	ntfs_inode *first_struct;
	
	
  
	char *ntfs3gdir = NULL;

	if (!ntfs_volume1 || !way) 
  {
  
		errno = EINVAL;
		return NULL;
    
	}
  
	ntfschar *char_code = NULL;
	ntfs_log_trace("path: '%s'\n", way);
	ntfs_inode *final_struct = NULL;
	ntfs3gdir = strdup(way);
  
  int sys_error = 0;
  
	if (!ntfs3gdir) 
  	{
		ntfs_log_error("Out of memory.\n");
		sys_error = ENOMEM;
			free(ntfs3gdir);
      free(char_code);
  
      sys_error = 0;  
	if (sys_error)
      {
		      errno = sys_error;
      }
	    return final_struct;
	}
  
  char *tuple;
	tuple = ntfs3gdir;
  
	
	while (tuple && *tuple && *tuple == PATH_SEP)
  {
    tuple++;
    
  }
  
if (ntfs_inode1) 
{
	first_struct = ntfs_inode1;
} else {
    	first_struct = ntfs_inode_open(ntfs_volume1, FILE_root);
    	if (!first_struct) 
    	{
		ntfs_log_debug("Couldn't open the inode of the root "
                     "directory.\n");
      			sys_error = EIO;
			final_struct = (ntfs_inode*)NULL;
			free(ntfs3gdir);
	   		free(char_code);
			if (sys_error)
			{
    				errno = sys_error;
  			}
    			return final_struct;
	}
}
char *quuee;
int lenght = 0;

while (tuple && *tuple) 
{
	quuee = strchr(tuple, PATH_SEP);
	if (quuee != NULL) 
    	{
		*quuee = '\0';
	}
	lenght = ntfs_mbstoucs(tuple, &char_code);
    
	if (lenght < 0) 
    	{
		ntfs_log_perror("Could not convert filename to Unicode:"
					" '%s'", tuple);
		sys_error = errno;
		if (first_struct && (first_struct != ntfs_inode1))
      		{
			if (ntfs_inode_close(first_struct) && !sys_error)
    			{
				sys_error = errno;
   			}
      		}
	} else if (lenght > NTFS_MAX_NAME_LEN) 
      	{
		sys_error = ENAMETOOLONG;
		if (first_struct && (first_struct != ntfs_inode1))
      		{
			if (ntfs_inode_close(first_struct) && !sys_error)
    			{
				sys_error = errno;
    			}
      		}
	}
	u64 number_64 = ntfs_inode_lookup_by_name(first_struct, char_code, lenght);
    	long unsigned int checker = ntfs_inode_lookup_by_name(first_struct, char_code, lenght);
		
    	if (checker == (u64) -1) 
    	{
		ntfs_log_debug("Couldn't find name '%s' in way "
					"'%s'.\n", tuple, way);
		sys_error = ENOENT;
		if (first_struct && (first_struct != ntfs_inode1))
      		{
			if (ntfs_inode_close(first_struct) && !sys_error)
    			{
				sys_error = errno;
    			}
      		}
   	}

	if (first_struct != ntfs_inode1)
    	{
		if (ntfs_inode_close(first_struct)) 
      		{
			sys_error = errno;
			free(ntfs3gdir);
			free(char_code);
			if (sys_error)
  			{
				errno = sys_error;
  			}
		return final_struct;
	}
    }

		number_64 = MREF(number_64);
		first_struct = ntfs_inode_open(ntfs_volume1, number_64);
    
		if (!first_struct) 
    {
			ntfs_log_debug("Cannot open inode %llu: %s.\n", (unsigned long long)number_64, tuple);
			sys_error = EIO;
			if (first_struct && (first_struct != ntfs_inode1))
      {
		if (ntfs_inode_close(first_struct) && !sys_error)
    {
			sys_error = errno;
    }
    }
    }
		if (quuee != NULL)
		{
		if (!(first_struct->mrec->flags & MFT_RECORD_IS_DIRECTORY))
			{
				sys_error = ENOTDIR;
		
        if (first_struct && (first_struct != ntfs_inode1))
        {
          if (ntfs_inode_close(first_struct) && !sys_error)
    {
			sys_error = errno;
    }
        }
    }
		}
	
		free(char_code);
		char_code = NULL;

		if (quuee)
    {
      *quuee++ = PATH_SEP;
    }
		tuple = quuee;
    
		while (tuple && *tuple && *tuple == PATH_SEP)
			tuple++;
	}

	final_struct = first_struct;
	first_struct = NULL;

	return final_struct;
}

int Id_file_name(int ID, char* arr)
{
	if (!arr)
  {
		return -1;
  }
	char finder[PATH_MAX] = {0};
	
  sprintf(finder, "/proc/self/fd/%d", ID);

	ssize_t val = readlink(finder, arr, PATH_MAX);
	
  if (val < 0)
  {
		return val;
  }
  
	arr[val] = '\0';
  
	return 0;
}

int dump_file(int img, const char *path, int out)
{
	

	

	ntfs_volume *vol = NULL;
	

	 const unsigned b_4096 = 4096;
  char data_cont[b_4096];

	  int result = 0;

	
	
  char path1[PATH_MAX];
	if (Id_file_name(img, path1) < 0)
		return -1;

	vol = ntfs_mount(fileName, NTFS_MNT_RDONLY);
	if (!vol)
		return -1;
	
 

  
	
  if (Id_file_name(img, path1) < 0)
  {
		return -1;
  }
  ntfs_attr* ntfs_attr1 = NULL;
  ntfs_volume *ntfs_volume1 = NULL;
	ntfs_inode *ntfs_inode = NULL;
  ntfs_volume1 = ntfs_mount(path1, NTFS_MNT_RDONLY);
	
  if (!ntfs_volume1)
  {
		return -1;
  }

  
  ntfs_inode = path_to_node(ntfs_volume1, NULL, path);
  
	
  if (!ntfs_inode)
	{
	result = -errno;
	ntfs_attr_close(ntfs_attr1);
	ntfs_inode_close(ntfs_inode);
	ntfs_umount(ntfs_volume1, FALSE);
	}

	
	ATTR_TYPES attrType = AT_DATA;
  
	ntfs_attr1 = ntfs_attr_open(ntfs_inode, attrType, NULL, 0);
	
  if (!ntfs_attr1)
	{
	result = -1;
	ntfs_attr_close(ntfs_attr1);
	ntfs_inode_close(ntfs_inode);
	ntfs_umount(ntfs_volume1, FALSE);
	}

	u32 bs = 0;
  
	if (ntfs_inode->mft_no < 2)
  {
		bs = ntfs_volume1->mft_record_size;
  }
  
	s64 ot = 0;
	s64 bR = 0;
  s64 red = 0;
  	int True = 1;
	while (True) 
	{
		if (bs > 0) 
		{
			bR = ntfs_attr_mst_pread(ntfs_attr1, ot, 1, bs, data_cont);
			if (bR > 0)
      {
			
        bR *= bs;
      }
      } 
		else 
		{
			bR = ntfs_attr_pread(ntfs_attr1, ot, 4096, data_cont);
		}

		if (bR < 0) 
		{
			result = -errno;
			ntfs_attr_close(ntfs_attr1);
	ntfs_inode_close(ntfs_inode);
	ntfs_umount(ntfs_volume1, FALSE);
		}

		if (!bR)
    {
			break; 
    }
		red = write(out, data_cont, bR);
		if (red != bR)
		{
			result = -errno;
			ntfs_attr_close(ntfs_attr1);
	    ntfs_inode_close(ntfs_inode);
	    ntfs_umount(ntfs_volume1, FALSE);
		}
		
		ot = ot + bR + 1;
    ot -= 1;
    //compilation problem
	}



	return result;
}
/*
int dump_file(int img, const char* way_const, int returner) 
{
  struct iovec* s1 = ci(sizeof(struct ext2_super_block));
  int checker = preadv(img, s1, 1, 1024);
 struct iovec* s3 = ci(sizeof(struct ext2_inode));
  if (checker < 0) {
  //if (read_superblock(img, super_block) < 0) {
    cler_i(s1);
    return errno;
  }

  struct iovec* s2 = ci(sizeof(struct ext2_group_desc));
 

  struct file_type* data_container = malloc(sizeof(struct file_type));
  data_container->returner = -1;
  data_container->matrix = 1;
  data_container->file_type1 = EXT2_FT_DIR;
  data_container->inode_nr = 1;

  if (way_const[0] == '/')
  {
	  ++way_const;
  }
  char file_name[256];
	
  size_t lght = take_way(file_name, way_const);
	
  while (lght > 0) 
  {
    int chocher = (strchr(way_const, '/') != NULL);
    strcpy(data_container->naimenovanie, file_name);
    way_const += lght;
    if (way_const[0] == '/') 
    {
	    ++way_const;
    }

    if (headway(s1, s2, s3, data_container, img) < 0) {
     free(data_container);
  cler_i(s3);
  cler_i(s1);
  cler_i(s2);
      return errno;
    }
    
    if (chocher && data_container->file_type1 != EXT2_FT_DIR) {
     free(data_container);
  cler_i(s3);
  cler_i(s1);
  cler_i(s2);
      return -ENOTDIR;
    }
    lght = take_way(file_name, way_const);
  }

  strcpy(data_container->naimenovanie, file_name);
  data_container->returner = returner;
  data_container->matrix = 2;

  if (headway(s1, s2, s3, data_container, img) < 0) {
    free(data_container);
  cler_i(s3);
  cler_i(s1);
  cler_i(s2);
    return errno;
  }

 free(data_container);
  cler_i(s3);
  cler_i(s1);
  cler_i(s2);
  return 0;
}*/
