#ifndef DISK_H
#define DISK_H

#define DISK_BLOCK_SIZE 4096

// Init the disk. Use the existing disk file specified if valid, otherwise open
// a new disk file. Initialize the disk to be nblocks * DISK_BLOCK_SIZE large.
// Returns 1 on success and 0 on failure
int disk_init( const char *filename, int nblocks );

// Returns the size of the disk in number of blocks
int disk_size();

// Reads one block of data from disk to the buffer provided. The buffer provided
// must be at least DISK_BLOCK_SIZE bytes large
// NOTE: Aborts on failure to read disk file
void disk_read( int blocknum, char *data );

// Writes one block of data from the buffer provided to disk
// NOTE: Aborts on failure to write disk file
void disk_write( int blocknum, const char *data );

// Close the disk file
void disk_close();

#endif
