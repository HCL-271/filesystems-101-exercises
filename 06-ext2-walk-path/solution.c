#include <ext2fs/ext2fs.h>
#include "solution.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/uio.h>





size_t urb = 0;

size_t bs = 0;



struct iovec* ci(size_t length) {
  struct iovec* iov = malloc(sizeof(struct iovec));
  
  iov->iov_base = malloc(length);
  iov->iov_len = length;

  return iov;
}








struct file_type {
  int matrix;
  int inode_nr;
  char naimenovanie[256];
  int file_type1;
  int returner;
};

void return_vvod(struct iovec* free_space, struct file_type* file_type2) {
  char naimenovanie[256];
  struct ext2_dir_entry_2* ext2_dir_entry_21;
  for (size_t i = 0; i < free_space->iov_len; i += ext2_dir_entry_21->rec_len) 
  {
    ext2_dir_entry_21 = (struct ext2_dir_entry_2*)(free_space->iov_base + i);
    memcpy(naimenovanie, ext2_dir_entry_21->name, ext2_dir_entry_21->name_len);
    naimenovanie[ext2_dir_entry_21->name_len] = '\0';

    if (strcmp(naimenovanie, file_type2->naimenovanie) == 0) {
      file_type2->inode_nr = ext2_dir_entry_21->inode - 1;
      file_type2->file_type1 = ext2_dir_entry_21->file_type;
    }
  }

}


int buffer_checker(int img, off_t offset1, struct iovec* iovec_mod, struct file_type* data_stuff) {
  size_t reading_prob = (urb < bs) ? urb : bs;
  iovec_mod->iov_len = reading_prob;
  int read_bytes = preadv(img, iovec_mod, 1, offset1);
  if (read_bytes < 0) 
  {
	  return errno;
		      }

  urb -= read_bytes;

  if (data_stuff->matrix == 1)
  {
    return_vvod(iovec_mod, data_stuff);
  }
	  else if (data_stuff->matrix == 2 && writev(data_stuff->returner, iovec_mod, 1) < 0)
	  {
		  return errno;
	  }
  return 0;
}

void cler_i(struct iovec* modification) {
  if (modification == NULL) 
  {
    return;
  }
  free(modification->iov_base);
  free(modification);
}
int rdb(int img, struct ext2_inode* ext2_inode1, struct file_type* data_conteiner) {
  struct iovec* memory = ci(bs);

  for (int i = 0; i < 12 && ext2_inode1->i_block[i] != 0; ++i) 
  {
    if (buffer_checker(img, ext2_inode1->i_block[i] * bs, memory, data_conteiner) < 0)
    {
      cler_i(memory);
      return errno;
    }

    if (urb == 0) 
    {
      break;
    }
  }
  cler_i(memory);
	

  return 0;
}

int rib(int img, uint32_t find_position, struct file_type* data_conteiner) {
  struct iovec* inside = ci(bs);
	
	int chck1= preadv(img, inside, 1, find_position);
  if (chck1 < 0) 
  {
    cler_i(inside);
    return errno;
  }

  uint32_t* stuff_place = inside->iov_base;

struct iovec* data_cont = ci(bs);
  for (size_t i = 0; i < bs / 4 && stuff_place[i] != 0; ++i) 
  {	
	  int chck = buffer_checker(img, stuff_place[i] * bs, data_cont, data_conteiner);
    if (chck < 0) 
    {
      cler_i(data_cont);
      return errno;
    }

    if (urb == 0) 
    {
      break;
    }
  }

  cler_i(data_cont);
  cler_i(inside);
	//cler_i(outside);
  return 0;
}

int handle_indir_block(int img, uint32_t distant_place,
                                struct file_type* data_conteiner) 
{
  struct iovec* two_inside = ci(bs);
	int zero = preadv(img, two_inside, 1, distant_place);
  if (zero < 0) 
  {
    cler_i(two_inside);
    return errno;
  }

  uint32_t* package_place = two_inside->iov_base;

  for (size_t i = 0; i < bs / 4 && package_place[i] != 0; ++i) 
  {
	  int aero1 = rib(img, package_place[i] * bs, data_conteiner);
    if (aero1 < 0)
    {
      return errno;
    }

    if (urb == 0) {
      break;
    }
  }
  cler_i(two_inside);
  return 0;
}

int transponse_data(int img, struct ext2_inode* ext2_inode1, struct file_type* data_container) {
  urb = ext2_inode1->i_size;
	//size_t urb1 = ext2_inode1->i_size;
  if (rdb(img, ext2_inode1, data_container) < 0) {
	//urb1++;
	  return errno;
  }
  int zero1 = rib(img, ext2_inode1->i_block[12] * bs,
                           data_container);
  if (zero1 < 0)
  { return errno;}

	int zero2 = handle_indir_block(img, ext2_inode1->i_block[12 + 1] * bs, data_container);
if (zero2 < 0)
  { 
	  return errno;
		//urb1++;
  }
  return 0;
}





int node_reader(int img, int before_pos, struct ext2_super_block* ext2_super_block1,
               struct ext2_group_desc* ext2_group_desc1, struct iovec* str_iovec1) 
{
  size_t inode_pos1 = before_pos % ext2_super_block1->s_inodes_per_group;

  off_t offset =
      ext2_group_desc1->bg_inode_table * bs + ext2_super_block1->s_inode_size * inode_pos1;
	
  int ret = preadv(img, str_iovec1, 1, offset);
  return ret;
}
int rgd1(int img, int integer1, struct ext2_super_block* ext2_super_block1,
                     struct iovec* descrptn) {
 bs = 1024 << ext2_super_block1->s_log_block_size;


  size_t element_poas = descrptn->iov_len * (integer1 / ext2_super_block1->s_inodes_per_group);

  off_t offset1 = bs + element_poas + ext2_super_block1->s_first_data_block * bs;
  int ret = preadv(img, descrptn, 1, offset1);
  return ret;
}

int headway(struct iovec* spsi, struct iovec* speaker_group,
             struct iovec* srteuct_iovec, struct file_type* data_container, int img) {
  int prevoius_node = data_container->inode_nr;
  data_container->inode_nr = -1;

  if (rgd1(img, prevoius_node, spsi->iov_base,
                       speaker_group) < 0 ||
      node_reader(img, prevoius_node, spsi->iov_base,
                 speaker_group->iov_base, srteuct_iovec) < 0 ||
      transponse_data(img, srteuct_iovec->iov_base, data_container) < 0){
	  return errno;
  }

  if (data_container->inode_nr == -1 && data_container->matrix == 1) {
    errno = -ENOENT;
    return errno;
  }
  return 0;
}



size_t take_way(char* destination, const char* way_cnst) 
{
  size_t i = 0;
	
  for (; way_cnst[i] != '\0' && way_cnst[i] != '/'; ++i)
  {
    destination[i] = way_cnst[i];
  }
	
  destination[i] = '\0';

  return i;
}


int dump_file(int img, const char* way_const, int returner) 
{
  struct iovec* s1 = ci(sizeof(struct ext2_super_block));
  int checker = preadv(img, s1, 1, 1024);
 struct iovec* s3 = ci(sizeof(struct ext2_inode));
  if (checker < 0) {

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
}
