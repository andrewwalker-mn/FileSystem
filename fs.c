#include "fs.h"
#include "disk.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#define FS_MAGIC           0xf0f03410
#define INODES_PER_BLOCK   128
#define POINTERS_PER_INODE 5
#define POINTERS_PER_BLOCK 1024

// Returns the number of dedicated inode blocks given the disk size in blocks
#define NUM_INODE_BLOCKS(disk_size_in_blocks) (1 + (disk_size_in_blocks / 10))

char* bitmap;
char mounted = 0;

struct fs_superblock {
	int magic;          // Magic bytes
	int nblocks;        // Size of the disk in number of blocks
	int ninodeblocks;   // Number of blocks dedicated to inodes
	int ninodes;        // Number of dedicated inodes
};

struct fs_inode {
	int isvalid;                      // 1 if valid (in use), 0 otherwise
	int size;                         // Size of file in bytes
	int direct[POINTERS_PER_INODE];   // Direct data block numbers (0 if invalid)
	int indirect;                     // Indirect data block number (0 if invalid)
};

union fs_block {
	struct fs_superblock super;               // Superblock
	struct fs_inode inode[INODES_PER_BLOCK];  // Block of inodes
	int pointers[POINTERS_PER_BLOCK];         // Indirect block of direct data block numbers
	char data[DISK_BLOCK_SIZE];               // Data block
};

void fs_debug()
{
	union fs_block block;

	disk_read(0,block.data);

	printf("superblock:\n");
  if (block.super.magic == FS_MAGIC) {
  	printf("    magic number is valid\n");
  }
  else {
    printf("    magic number is NOT valid\n");
  }
	printf("    %d blocks on disk\n",block.super.nblocks);
	printf("    %d inode blocks\n",block.super.ninodeblocks);
	printf("    %d inodes\n",block.super.ninodes);


	int inumber = 0;
	for(int i = 1; i < block.super.ninodeblocks+1; i++) {
		union fs_block newblock;
		disk_read(i, newblock.data);

		for(int j = 0; j < INODES_PER_BLOCK; j++ ){
			if(newblock.inode[j].isvalid == 1) {
				printf("inode %d:\n", inumber);
				printf("    size: %d bytes\n", newblock.inode[j].size);
				printf("    direct blocks: ");
				for(int k = 0; k < POINTERS_PER_INODE; k++) {
					if (newblock.inode[j].direct[k] != 0) {
						printf("%d ", newblock.inode[j].direct[k]);
					}
				}
				printf("\n");
        if (newblock.inode[j].indirect) {
          printf("    indirect block: %d\n", newblock.inode[j].indirect);
          union fs_block indirectBlock;
          disk_read(newblock.inode[j].indirect, indirectBlock.data);
    
          printf("    indirect data blocks: ");
          for(int l = 0; l < POINTERS_PER_BLOCK; l++) {
            if (indirectBlock.pointers[l] != 0) {
              printf("%d ", indirectBlock.pointers[l]);
            }
          }
          printf("\n");
        }
			}
			inumber += 1;
		}
        
    //TODO (future): This is also a great place to assert invariants in your 
    // filesystem to make sure the disk matches with your expectation 
    // and any in-memory structures you have (freemap). 
    
    //TODO (future): Filesystem commands should return failure if attempting to 
    // perform operations that require the filesystem to be 
    // mounted/unmounted (refer to solution exe). Check against solution exe
	}
}

int fs_format()
{
  // create a new filesystem on the disk, destroying any data already present
  int size = disk_size();
  char zeroed_block[DISK_BLOCK_SIZE];
  memset(zeroed_block, 0, sizeof(zeroed_block));
  
  for (int i=0; i<disk_size(); i++) {
    disk_write(i, zeroed_block);
  }
    
  // set aside 10% of the blocks for inodes, clear the inode table
	// write the superblock
	union fs_block block;
	block.super.magic = FS_MAGIC;
	block.super.nblocks = size;

	int numinodeblock = NUM_INODE_BLOCKS(size);

	block.super.ninodeblocks = numinodeblock;
	block.super.ninodes = numinodeblock * INODES_PER_BLOCK;

	disk_write(0, block.data);
	return 1;
}

int fs_mount()
// build free block bitmap by scanning through all inodes and seeing which are in use
{
  if (mounted) {
    printf("Unmount before remounting");
		return 0;
	}
  
	union fs_block block;

	disk_read(0, block.data);
	bitmap = malloc(block.super.nblocks);
	bitmap[0] = 1;

	if(block.super.magic == FS_MAGIC) {
		// iterate through inode blocks
		for(int i = 1; i < block.super.ninodeblocks+1; i++) {
				union fs_block newblock;
				disk_read(i, newblock.data);
        bitmap[i] = 1;
        
				// for each inode block iterate through its inodes (there should be 128)
				for(int j = 0; j < INODES_PER_BLOCK; j++) {
					if(newblock.inode[j].isvalid == 1) {

						for(int k = 0; k < POINTERS_PER_INODE; k++) {
							if (newblock.inode[j].direct[k] != 0) {
								bitmap[newblock.inode[j].direct[k]] = 1;
							}
						}
						if (newblock.inode[j].indirect != 0) {
              bitmap[newblock.inode[j].indirect] = 1;
							union fs_block pointers;
							disk_read(newblock.inode[j].indirect, pointers.data);
							for(int k = 0; k < POINTERS_PER_BLOCK; k++) {
								if (pointers.pointers[k] != 0) {
									bitmap[pointers.pointers[k]] = 1;
								}
							}
						}
					}
				}

		}
		mounted = 1;
		return 1;
	}

	return 0;
}

int fs_unmount()
{
	if (!mounted) {
    printf("Not currently mounted\n");
		return 0;
	}
	free(bitmap);
	mounted = 0;
	return 1;
}

int fs_create()
{
	// when created, choose first available inumber and return it to user.

	union fs_block block;
	disk_read(0, block.data);

	int inumber = 0;

	if(!mounted) {
    printf("Must be mounted to perform create\n");
		return -1;
	}

	for(int i = 1; i < block.super.ninodeblocks+1; i++) {
			union fs_block newblock;
			disk_read(i, newblock.data);

			for(int j = 0; j < INODES_PER_BLOCK; j++) {

				if(newblock.inode[j].isvalid != 1) {
					struct fs_inode inode;
					inode.isvalid = 1;
					inode.size = 0;
					for(int k = 0; k < POINTERS_PER_INODE; k++ ) {
						inode.direct[k] = 0;
					}
          inode.indirect = 0;
					newblock.inode[j] = inode;
					disk_write(i, newblock.data);
					return inumber;
				}
				inumber += 1;
			}

	}

	return -1;
}

int getBlockNumFromInodeNum(int inumber) {
  //Calculates the block that the inode is saved in
  return inumber/INODES_PER_BLOCK + 1;
}

int getOffsetFromInodeNum(int inumber) {
  //Calculates the offset into a block that the inode is saved in
  return inumber % INODES_PER_BLOCK;
}

int fs_delete( int inumber )
{
  // union fs_block block;
  // disk_read(0, block.data);
  
  if (!mounted) {
    printf("Must be mounted to perform delete\n");
    return 0;
  }
  
  union fs_block newblock;
  disk_read(getBlockNumFromInodeNum(inumber), newblock.data);
  int j = getOffsetFromInodeNum(inumber);
  if (newblock.inode[j].isvalid != 1) {
    return 0;
  }
  
  //we dont have to write over with 0's, do we?
  union fs_block pointers;
  disk_read(newblock.inode[j].indirect, pointers.data);
  
  if (newblock.inode[j].indirect != 0) {
    for(int k = 0; k < POINTERS_PER_BLOCK; k++) {
      if (bitmap[pointers.pointers[k]] = 1) {
        pointers.pointers[k] = 0;
        bitmap[pointers.pointers[k]] = 0;
      }
    }
    bitmap[newblock.inode[j].indirect] = 0;
    newblock.inode[j].indirect = 0;
  }
  
  for(int k = 0; k < POINTERS_PER_INODE; k++) {
    if (bitmap[newblock.inode[j].direct[k]] = 1) {
      newblock.inode[j].direct[k] = 0;
      bitmap[newblock.inode[j].direct[k]] = 0;
    }
  }
  newblock.inode[j].isvalid = 0;
  disk_write(getBlockNumFromInodeNum(inumber), newblock.data);
  return 1;
}

int fs_getsize( int inumber )
{
  if (!mounted) {
    printf("Must be mounted to getsize");
    return -1
  }
  union fs_block newblock;
  disk_read(getBlockNumFromInodeNum(inumber), newblock.data);
  if (newblock.inode[getOffsetFromInodeNum(inumber)].isvalid) {
  return newblock.inode[getOffsetFromInodeNum(inumber)].size;
  }
  
	return -1;
}

int fs_read( int inumber, char *data, int length, int offset )
{
	return 0;
}

int fs_write( int inumber, const char *data, int length, int offset )
{
	return 0;
}
