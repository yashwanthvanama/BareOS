#include <barelib.h>
#include <fs.h>
#include <string.h>

extern fsystem_t* fsd;
extern filetable_t oft[NUM_FD];

/*  Search for a filename  in the directory, if the file doesn't exist  *
 *  or it is  already  open, return  an error.   Otherwise find a free  *
 *  slot in the open file table and initialize it to the corresponding  *
 *  inode in the root directory.                                        *
 *  'head' is initialized to the start of the file.                     */
int32 fs_open(char* filename) {
  int i, j;
  for(i = 0; i < fsd->root_dir.numentries && !strcmp(fsd->root_dir.entry[i].name, filename); i++);
  if(!strcmp(fsd->root_dir.entry[i].name, filename))
    return -1;
  for(j = 0; j < NUM_FD && !strcmp(fsd->root_dir.entry[(oft[j].direntry)].name, filename); j++);
  if(strcmp(fsd->root_dir.entry[(oft[j].direntry)].name, filename) && oft[j].state == FSTATE_OPEN)
    return -1;
  inode_t file_inode;
  bs_read(fsd->root_dir.entry[i].inode_block, 0, &file_inode, sizeof(inode_t));
  if(strcmp(fsd->root_dir.entry[(oft[j].direntry)].name, filename) && oft[j].state == FSTATE_CLOSED);
  else {
    for(j = 0; j < NUM_FD && oft[j].state == FSTATE_OPEN; j++);
    if(j == NUM_FD)
      return -1;
  }
  oft[j].direntry = i;
  oft[j].state = FSTATE_OPEN;
  oft[j].inode = file_inode;
  oft[j].head = 0;
  return j;
}
