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
	printf("    %d blocks\n",block.super.nblocks);
	printf("    %d inode blocks\n",block.super.ninodeblocks);
	printf("    %d inodes\n",block.super.ninodes);
}

int fs_format()
{
	return 0;
}

int fs_mount()
{
	return 0;
}

int fs_unmount()
{
	return 0;
}

int fs_create()
{
	return -1;
}

int fs_delete( int inumber )
{
	return 0;
}

int fs_getsize( int inumber )
{
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
