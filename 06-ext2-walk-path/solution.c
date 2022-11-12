#include <ext2fs/ext2fs.h>
#include <solution.h>
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








struct common {
  int traverse_mode;
  int inode_nr;
  char name[256];
  int file_type;
  int out;
};
/*
int path_finder(int fd, int i_num, char* cahr, const struct ext2_super_block* sb)
{
	struct ext2_inode inode = {};
	size_t block_size = 1024 << sb->s_log_block_size;
	struct ext2_group_desc gggdd = {};
  bs = 1024 << sb->s_log_block_size;

  size_t bpbpbp = descr->iov_len * (inode_nr / sb->s_inodes_per_group);

  off_t bpbpaaabp = bs + bpbpbp + sb->s_first_data_block * bs;
	if ((i_num == 2) && ((inode.i_mode & LINUX_S_IFDIR) == 0))
		return -ENOTDIR;
	off_t gg = SUPERBLOCK_OFFSET + sizeof(*sb) + (i_num - 1) / sb->gggdd * sizeof(group_desc);
	pread(fd, &group_desc, sizeof(group_desc), gd_offset);

	pread(fd, &inode, sizeof(inode), block_size * bpbpbp.bg_inode_table + (inode_number - 1) % sb->block_size * sb->block_size);

	if ((i_num == 2) && ((gggdd.i_mode & LINUX_S_IFDIR) == 0))
		return -ENOTDIR;
	char* ppth = strchr(path, '/');
	if (next_path != NULL)
	{
		if ((gggdd.i_mode & LINUX_S_IFDIR) == 0)
			return -ENOTDIR;
		*(ppth++) = '\0';
		int run = gipb(fd, gg, block_size, &inode);
		if (run < 0)
			return -ENOTDIR;
		return gipb(fd, ppth, cahr, sb);
	}
	else
		return get_inode(fd, path, block_size, &inode);
}
*/
void report_entry(struct iovec* buf, struct common* file) {
  char name[256];
  struct ext2_dir_entry_2* dir_entry;
  for (size_t i = 0; i < buf->iov_len; i += dir_entry->rec_len) {
    dir_entry = (struct ext2_dir_entry_2*)(buf->iov_base + i);
    memcpy(name, dir_entry->name, dir_entry->name_len);
    name[dir_entry->name_len] = '\0';

    if (strcmp(name, file->name) == 0) {
      file->inode_nr = dir_entry->inode - 1;
      file->file_type = dir_entry->file_type;
    }
  }
}

int read_block(int img, off_t offset, struct iovec* buf, struct common* file) {
  size_t should_read = (urb < bs) ? urb : bs;
  buf->iov_len = should_read;
  int read_bytes = preadv(img, buf, 1, offset);
  if (read_bytes < 0) return errno;

  urb -= read_bytes;

  if (file->traverse_mode == 1)
    report_entry(buf, file);
  else if (file->traverse_mode == 2 && writev(file->out, buf, 1) < 0)
    return errno;
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
int read_direct_blocks(int img, struct ext2_inode* inode, struct common* file) {
  struct iovec* buf = ci(bs);
  for (int i = 0; i < 12 && inode->i_block[i] != 0; ++i) {
    if (read_block(img, inode->i_block[i] * bs, buf, file) < 0) {
      cler_i(buf);
      return errno;
    }

    if (urb == 0) {
      break;
    }
  }
  cler_i(buf);
  return 0;
}

int read_indirect_blocks(int img, uint32_t ind_pos, struct common* file) {
  struct iovec* indirect = ci(bs);
  if (preadv(img, indirect, 1, ind_pos) < 0) {
    cler_i(indirect);
    return errno;
  }
  uint32_t* blocks_pos = indirect->iov_base;

  struct iovec* buf = ci(bs);
  for (size_t i = 0; i < bs / 4 && blocks_pos[i] != 0; ++i) {
    if (read_block(img, blocks_pos[i] * bs, buf, file) < 0) {
      cler_i(buf);
      return errno;
    }

    if (urb == 0) {
      break;
    }
  }

  cler_i(buf);
  cler_i(indirect);
  return 0;
}

int read_double_indirect_blocks(int img, uint32_t dint_pos,
                                struct common* file) {
  struct iovec* double_indirect = ci(bs);
  if (preadv(img, double_indirect, 1, dint_pos) < 0) {
    cler_i(double_indirect);
    return errno;
  }

  uint32_t* blocks_pos = double_indirect->iov_base;

  for (size_t i = 0; i < bs / 4 && blocks_pos[i] != 0; ++i) {
    if (read_indirect_blocks(img, blocks_pos[i] * bs, file) < 0) {
      return errno;
    }

    if (urb == 0) {
      break;
    }
  }
  cler_i(double_indirect);
  return 0;
}

int traverse_inode(int img, struct ext2_inode* inode, struct common* file) {
  urb = inode->i_size;

  if (read_direct_blocks(img, inode, file) < 0) return errno;
  if (read_indirect_blocks(img, inode->i_block[12] * bs,
                           file) < 0)
    return errno;
  if (read_double_indirect_blocks(
          img, inode->i_block[12 + 1] * bs, file) < 0)
    return errno;

  return 0;
}


int read_group_descr(int img, int inode_nr, struct ext2_super_block* sb,
                     struct iovec* descr) {
  block_size = 1024 << sb->s_log_block_size;

  size_t block_pos = descr->iov_len * (inode_nr / sb->s_inodes_per_group);

  off_t offset = block_size + block_pos + sb->s_first_data_block * block_size;

  return preadv(img, descr, 1, offset);
}


int read_inode(int img, int before_inode, struct ext2_super_block* sb,
               struct ext2_group_desc* descr, struct iovec* inode) {
  size_t inode_pos = inode_nr % sb->s_inodes_per_group;

  off_t offset =
      descr->bg_inode_table * bs + sb->s_inode_size * inode_pos;

  return preadv(img, inode, 1, offset);
}

int traverse(struct iovec* super_block, struct iovec* group_descriptor,
             struct iovec* inode, struct common* file, int img) {
  int before_inode = file->inode_nr;
  file->inode_nr = -1;
  
 
    size_t bs1 = 1024 << (super_block->iov_base)->s_log_block_size;

  size_t block_pos = group_descriptor->iov_len * (inode_modification / (super_block->iov_base)->s_inodes_per_group);

  off_t offset = bs1 + block_pos + (super_block->iov_base)->s_first_data_block * bs1;
   int checker1 = preadv(img, group_descriptor, 1, offset);
  
  size_t inode_pos = inode_nr % (super_block->iov_base)->s_inodes_per_group;

  off_t offset1 =
      (group_descriptor->iov_base)->bg_inode_table * bs + (super_block->iov_base)->s_inode_size * inode_pos;

  int checker2 =  preadv(img, inode, 1, offset1);
  /*
  if (read_group_descr(img, before_inode, super_block->iov_base,
                       group_descriptor) < 0 ||
      read_inode(img, before_inode, super_block->iov_base,
                 group_descriptor->iov_base, inode) < 0 ||
      traverse_inode(img, inode->iov_base, file) < 0)
      */
  if (checker1< 0 ||
      checker2 < 0 ||
      traverse_inode(img, inode->iov_base, file) < 0)
    return errno;

  if (file->inode_nr == -1 && file->traverse_mode == 1) {
    errno = -ENOENT;
    return errno;
  }
  return 0;
}

size_t get_root(char* dest, const char* path) {
  size_t i = 0;
  for (; path[i] != '\0' && path[i] != '/'; ++i) {
    dest[i] = path[i];
  }
  dest[i] = '\0';
  // if (path[i] == '\0') return 0;
  return i;
}

void clear(struct iovec* super_block, struct iovec* group_descriptor,
           struct iovec* inode, struct common* file) {
  free(file);
  cler_i(inode);
  cler_i(super_block);
  cler_i(group_descriptor);
}
int read_superblock(int img, struct iovec* sb) {
  return preadv(img, sb, 1, 1024);
}
int dump_file(int img, const char* path, int out) {
  struct iovec* super_block = ci(sizeof(struct ext2_super_block));
  int checker = preadv(img, sb, 1, 1024);
  if (checker < 0) {
  //if (read_superblock(img, super_block) < 0) {
    cler_i(super_block);
    return errno;
  }

  struct iovec* group_descriptor = ci(sizeof(struct ext2_group_desc));
  struct iovec* inode = ci(sizeof(struct ext2_inode));

  struct common* file = malloc(sizeof(struct common));
  file->out = -1;
  file->traverse_mode = 1;
  file->file_type = EXT2_FT_DIR;
  file->inode_nr = 1;

  if (path[0] == '/') ++path;
  char file_name[256];
  size_t name_len = get_root(file_name, path);
  while (name_len > 0) {
    int is_dir = (strchr(path, '/') != NULL);
    strcpy(file->name, file_name);
    path += name_len;
    if (path[0] == '/') ++path;

    if (traverse(super_block, group_descriptor, inode, file, img) < 0) {
      clear(super_block, group_descriptor, inode, file);
      return errno;
    }
    
    if (is_dir && file->file_type != EXT2_FT_DIR) {
      clear(super_block, group_descriptor, inode, file);
      return -ENOTDIR;
    }
    name_len = get_root(file_name, path);
  }

  strcpy(file->name, file_name);
  file->out = out;
  file->traverse_mode = 2;

  if (traverse(super_block, group_descriptor, inode, file, img) < 0) {
    clear(super_block, group_descriptor, inode, file);
    return errno;
  }

  clear(super_block, group_descriptor, inode, file);
  return 0;
}
