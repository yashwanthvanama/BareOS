#include <barelib.h>
#include <fs.h>
#include <string.h>
#include <bareio.h>

extern fsystem_t* fsd;

/*  Search for 'filename' in the root directory.  If the  *
 *  file exists,  returns an error.  Otherwise, create a  *
 *  new file  entry in the  root directory, a llocate an  *
 *  unused  block in the block  device and  assign it to  *
 *  the new file.                                         */
int32 fs_create(char* filename) {
  if(fsd->root_dir.numentries == DIR_SIZE)
    return -1;
  for(int i = 0; i < fsd->root_dir.numentries; i++) {
    if(strcmp(fsd->root_dir.entry[i].name, filename))
      return -1;
  }
  int i;
  for(i = 0; fsd->root_dir.entry[i].inode_block != EMPTY; i++);
  int j;
  for(j = 0; j < MDEV_NUM_BLOCKS && fs_getmaskbit(j) != 0; j++);
  if(j == MDEV_NUM_BLOCKS)
    return -1;
  fs_setmaskbit(j);
  inode_t inode;
  inode.id = j;
  inode.size = 0;
  for(int k = 0; k < INODE_BLOCKS; k++) {
    inode.blocks[k] = 0;
  }
  fsd->root_dir.entry[i].inode_block = j;
  fsd->root_dir.numentries++;
  strcpy(fsd->root_dir.entry[i].name, filename);
  fsd->freemask[j / 8] |= 0x1 << (j % 8);
  bs_write(j, 0, &inode, sizeof(inode_t));
  bs_write(BM_BIT, 0, fsd->freemask, fsd->freemasksz);
  return 0;
}
