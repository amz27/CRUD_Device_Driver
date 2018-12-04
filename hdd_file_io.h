#ifndef HDD_FILE_IO_INCLUDED
#define HDD_FILE_IO_INCLUDED

// Include files
#include <stdint.h>

//
// Interface functions

int16_t hdd_open(char *path);
	// This function opens the file and returns a file handle

int16_t hdd_close(int16_t fd);
	// This function closes the file

int32_t hdd_read(int16_t fd, void *buf, int32_t count);
	// Reads "count" bytes from the file handle "fh" into the buffer  "buf"

int32_t hdd_write(int16_t fd, void *buf, int32_t count);
	// Writes "count" bytes to the file handle "fh" from the buffer  "buf"

int32_t hdd_seek(int16_t fd, uint32_t loc);
	// Seek to specific point in the file

//
// Unit testing for the module

int hddIOUnitTest(void);
	// Perform a test of the CRUD IO implementation

#endif


