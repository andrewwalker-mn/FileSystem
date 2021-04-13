#ifndef FS_H
#define FS_H

// Print debug information about the file system. Also a great location to
// assert file system invariants
void fs_debug();

// Format the file system by initializing the superblock and inodes on disk
// Returns 1 on success and 0 on failure
int fs_format();

// Mount the file system by initializing the freemap based on the current state
// of the disk
// Returns 1 on success and 0 on failure
int fs_mount();

// Unmount the file system
// Returns 1 on success and 0 on failure
int fs_unmount();

// Create an empty file
// Returns newly allocated inode number (>= 0) on success and -1 on failure
int fs_create();

// Delete the file given by the specified inode number
// Returns 1 on success and 0 on failure
int fs_delete( int inumber );

// Get the size of the file given by the specified inode number
// Returns file size on success and -1 on failure
int fs_getsize( int inumber );

// Read [offset, offset + length) bytes of the file specified by inumber into
// the data buffer provided. The data buffer should be at least "length" long.
// If offset is withing the bounds of the file but offset+length is off the end
// of the file then only read to the end of the file
// Returns bytes read (> 0) on success and 0 on failure
int fs_read( int inumber, char *data, int length, int offset );

// Write length bytes of the data buffer provided to the file specified by
// inumber at offset. If length+offset goes beyond the max length of a file
// then only write the bytes that fit given the max file size
// Returns bytes written (> 0) on success and 0 on failure
int fs_write( int inumber, const char *data, int length, int offset );

#endif
