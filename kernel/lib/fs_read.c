#include <barelib.h>
#include <fs.h>
#include <bareio.h>

/* fs_read - Takes a file descriptor index into the 'oft', a  pointer to a  *
 *           buffer that the function writes data to and a number of bytes  *
 *           to read.                                                       *
 *                                                                          *
 *           'fs_read' reads data starting at the open file's 'head' until  *
 *           it has  copied either 'len' bytes  from the file's  blocks or  *
 *           the 'head' reaches the end of the file.                        *
 *                                                                          *
 * returns - 'fs_read' should return the number of bytes read (either 'len' *
 *           or the  number of bytes  remaining in the file,  whichever is  *
 *           smaller).                                                      */
uint32 fs_read(uint32 fd, char* buff, uint32 len) {
  int i = 0;
  int blocks = (oft[fd].inode.size / MDEV_BLOCK_SIZE) + 1;
  int current_block = oft[fd].head / MDEV_BLOCK_SIZE;
  int j = oft[fd].head % MDEV_BLOCK_SIZE;
  char inter_buff[MDEV_BLOCK_SIZE];
  while(i < len && current_block < blocks) {
    bs_read(oft[fd].inode.blocks[current_block], 0, inter_buff, MDEV_BLOCK_SIZE);
    while(j < MDEV_BLOCK_SIZE && i < len && oft[fd].head < oft[fd].inode.size){
      buff[i] = inter_buff[j];
      j++;
      i++;
      oft[fd].head++;
    }
    j = 0;
    current_block++;
  }
  return i;
}
