#include <barelib.h>
#include <fs.h>

/* fs_write - Takes a file descriptor index into the 'oft', a  pointer to a  *
 *            buffer  that the  function reads data  from and the number of  *
 *            bytes to copy from the buffer to the file.                     *
 *                                                                           *
 *            'fs_write' reads data from the 'buff' and copies it into the   *
 *            file  'blocks' starting  at the 'head'.  The  function  will   *
 *            allocate new blocks from the block device as needed to write   *
 *            data to the file and assign them to the file's inode.          *
 *                                                                           *
 *  returns - 'fs_write' should return the number of bytes written to the    *
 *            file.                                                          */
uint32 fs_write(uint32 fd, char* buff, uint32 len) {
  int j, k = 0;
  int bytes_written = 0;
  int bytes_to_write = len;
  int blocks = (oft[fd].inode.size / MDEV_BLOCK_SIZE);
  if(oft[fd].head % MDEV_BLOCK_SIZE != 0) {
    int block_head = oft[fd].head % MDEV_BLOCK_SIZE;
    j = k = oft[fd].head / MDEV_BLOCK_SIZE;
    bs_write(oft[fd].inode.blocks[j], block_head, buff, (MDEV_BLOCK_SIZE - block_head) < bytes_to_write ? (MDEV_BLOCK_SIZE - block_head) : bytes_to_write);
    oft[fd].inode.size += (MDEV_BLOCK_SIZE - block_head) < bytes_to_write ? (MDEV_BLOCK_SIZE - block_head) : bytes_to_write;
    bytes_written += (MDEV_BLOCK_SIZE - block_head) < bytes_to_write ? (MDEV_BLOCK_SIZE - block_head) : bytes_to_write;
    oft[fd].head += (MDEV_BLOCK_SIZE - block_head) < bytes_to_write ? (MDEV_BLOCK_SIZE - block_head) : bytes_to_write;
    bytes_to_write -= (MDEV_BLOCK_SIZE - block_head) < bytes_to_write ? (MDEV_BLOCK_SIZE - block_head) : bytes_to_write;
  }
  while(bytes_to_write > 0){
    if(k < blocks) {
      j = oft[fd].inode.blocks[k];
      k++;
    }
    else {
      for(j = 0; j < MDEV_NUM_BLOCKS && fs_getmaskbit(j) != 0; j++);
      if(j == MDEV_NUM_BLOCKS)
        return bytes_written;
      oft[fd].inode.blocks[(oft[fd].inode.size / MDEV_BLOCK_SIZE)] = j;
      oft[fd].inode.size += MDEV_BLOCK_SIZE < bytes_to_write ? MDEV_BLOCK_SIZE : bytes_to_write;
    }
    bs_write(j, 0, buff + bytes_written, MDEV_BLOCK_SIZE < bytes_to_write ? MDEV_BLOCK_SIZE : bytes_to_write);
    bytes_written += MDEV_BLOCK_SIZE < bytes_to_write ? MDEV_BLOCK_SIZE : bytes_to_write;
    oft[fd].head += MDEV_BLOCK_SIZE < bytes_to_write ? MDEV_BLOCK_SIZE : bytes_to_write;
    bytes_to_write -= MDEV_BLOCK_SIZE < bytes_to_write ? MDEV_BLOCK_SIZE : bytes_to_write;
    fsd->freemask[j / 8] |= 0x1 << (j % 8);
    bs_write(BM_BIT, 0, fsd->freemask, fsd->freemasksz);
    bs_write(oft[fd].inode.id, 0, &oft[fd].inode, sizeof(inode_t));
  }
  return bytes_written;
}
