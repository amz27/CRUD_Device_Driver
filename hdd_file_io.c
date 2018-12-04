////////////////////////////////////////////////////////////////////////////////
//
//  File           : hdd_file_io.h
//  Description    : This is the implementation of the standardized IO functions
//                   for used to access the HDD storage system.
//
//  Author         : Patrick McDaniel
//  Last Modified  : Sat Sep 2nd 08:56:10 EDT 2017
//
////////////////////////////////////////////////////////////////////////////////
//
// STUDENTS MUST ADD COMMENTS BELOW and FILL INT THE CODE FUNCTIONS
//

// Includes
#include <malloc.h>
#include <string.h>

// Project Includes
#include <hdd_file_io.h>
#include <hdd_driver.h>
#include <hdd_driver.h>
#include <cmpsc311_log.h>
#include <cmpsc311_util.h>


// Defines (you can ignore these)
#define MAX_HDD_FILEDESCR 1024
#define HIO_UNIT_TEST_MAX_WRITE_SIZE 1024
#define HDD_IO_UNIT_TEST_ITERATIONS 10240


// Type for UNIT test interface (you can ignore this)
typedef enum {
	HIO_UNIT_TEST_READ   = 0,
	HIO_UNIT_TEST_WRITE  = 1,
	HIO_UNIT_TEST_APPEND = 2,
	HIO_UNIT_TEST_SEEK   = 3,
} HDD_UNIT_TEST_TYPE;

// Global structure to stored metadata
typedef struct globalData{
	int16_t fHandler;
	int32_t curPosition;
	uint64_t blockId;
	uint64_t blockSize;
} fileInfo;

// Global variables
fileInfo file;
int uniqInt = 1;
HddBitCmd command = 0;
HddBitResp returnVal;

// Helper function for creating block that returns blockId
uint64_t createBlock(void *data, int32_t count){
	command = (uint64_t)HDD_BLOCK_CREATE;
	command = (command | count) << 36;
	command = command | 0;
	returnVal = hdd_data_lane(command, data);
	return returnVal;
}

// Helper function for reading block that returns a generated command
uint64_t readBlock(){
	command = (uint64_t)HDD_BLOCK_READ << 62;
	command = command | ((uint64_t)(file.blockSize) << 36);
	command = command | (file.blockId);
	return command;

}

// Helper function to overwrite block that returns a generated command
uint64_t overwriteBlock(){
	command = (uint64_t)HDD_BLOCK_OVERWRITE << 62;
	command = command | ((uint64_t)(file.blockSize) << 36);
	command = command | (file.blockId);		
	return command;
}

//
// Implementation

////////////////////////////////////////////////////////////////////////////////
//
// Function     : hdd_open
// Description  : open the file, initialize block, set metadata in global struct
//
// Inputs       : path
// Outputs      : unique integer file handler on success or -1 on failure
//
int16_t hdd_open(char *path) {
	// check for request file
	// initialize block on success
	if(path != NULL){
		if(hdd_initialize()){
			printf("Initialize block successful\n");
		}
		else{
			printf("Initialize block fail\n");
			return -1;
		}
		file.fHandler = uniqInt;
		file.blockId = 0;
		file.curPosition = 0;
		return uniqInt;
	}
	// return -1 on failure
	else{
		printf("Fail to open the request file\n");
        	return -1;
	}

}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : hdd_close
// Description  : close the file referenced by the file handler, delete content stored in the block
//
// Inputs       : file handler 
// Outputs      : 1 on success or -1 on failure
//
int16_t hdd_close(int16_t fh) {
	// check for file handler
	// delete block on success
	if(fh == file.fHandler){
		hdd_delete_block(file.blockId);	
		return 0;
	}
	// return -1 on failue
	else{
		printf("The file is not openning");
		return -1;
	}
	
}

////////////////////////////////////////////////////////////////////////////////
// Function     : hdd_read
// Description  : reads a count number of bytes from current position and place them into the buffer
//
// Inputs       : file handler, data buffer, a count number of bytes
// Outputs      : -1 if file is not opening or a count number of bytes on success
//
int32_t hdd_read(int16_t fh, void * data, int32_t count) {
	// check if the file is openning
	if(fh != file.fHandler){
		printf("The file is not openning\n");
		return -1;
	}

	// check if the block to read from is existing
	if(file.blockId == 0){
		printf("No block created\n");
		return -1;
	}
	
	// read as many bytes that are avaliable and place them into data buffer
	if(file.curPosition + count > file.blockSize){
		// allocate memory for data
		char *newData;
		newData = (char*)malloc(file.blockSize);
		
		// call readBlock and copy content into the new data buffer
		uint64_t rcmd = readBlock();
		hdd_data_lane(rcmd, newData);

		// copy content from the current position to data buffer
		memcpy(data, &newData[file.curPosition], file.blockSize - file.curPosition);

		// return the expected count of bytes
		int32_t expectedCount = file.blockSize - file.curPosition;
		file.curPosition = file.blockSize;
		free(newData);
		return expectedCount;
	}

	// read count number of bytes and place them into data buffer
	else{
		// allocate memory for data
		char *newData; 
		newData = (char*)malloc(file.blockSize);
		
		// call readBlock and copy content into the new data buffer
		uint64_t rcmd = readBlock();
		hdd_data_lane(rcmd, newData);

		// copy content to data buffer
		memcpy(data, &newData[file.curPosition], count);

		// return count
		file.curPosition += count;
		free(newData);
		return count;
	}

}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : hdd_write
// Description  : writes a count number of bytes at current postion from buffer data
//
// Inputs       : file handler, data buffer, a count of bytes
// Outputs      : -1 if file is not openning or a count of bytes on success
//
int32_t hdd_write(int16_t fh, void *data, int32_t count) {
	// check if the request file is openning
	if(fh != file.fHandler){
		printf("The file is not openning");
		return -1;
	}

	// create a new block if no block exist
	if(file.blockId == HDD_NO_BLOCK){
		returnVal = createBlock(data, count);
		file.curPosition = count;
		file.blockId = returnVal & 0xffffffff;
		file.blockSize = hdd_read_block_size(file.blockId);
		return count;

	}
	else{
		// when number of bytes to written is less than the existing block
		// reads the content to data buffer and overwrite to the block
		if(count + file.curPosition <= file.blockSize){
			char *newData;
			newData = (char*)malloc(file.blockSize);

		// call helper function readBlock
			uint64_t rcmd = readBlock();
			hdd_data_lane(rcmd, newData);
			memcpy(&newData[file.curPosition], data, count);
		
		// call helper function overwriteBlock
			uint64_t wcmd = overwriteBlock();
			hdd_data_lane(wcmd, newData);
			file.curPosition = file.curPosition + count;
			free(newData);
			return count;
		}

		// when number of bytes to written extends beyond the size of the block
		// create a new block of a larger size
		else{
			// allocate memory
			char *newData; 
			newData = (char*)malloc(file.curPosition + count);

			// call readBlock
			uint64_t  rcmd = readBlock();
			hdd_data_lane(rcmd, newData);

			// copy content 
			memcpy(&newData[file.curPosition], data, count);
			
			// create new block and get blockId
			returnVal = createBlock(newData, file.curPosition + count);
			
			// delete the old block
			hdd_delete_block(file.blockId);

			// update metadata
			file.curPosition = file.curPosition + count;
			file.blockId = returnVal & 0xffffffff;
			file.blockSize = hdd_read_block_size(file.blockId);			
			free(newData);	
			return count;
		}
	
	}
	return -1;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : hdd_seek
// Description  : changes the current seek position of file to loc
//
// Inputs       : file handler, loc
// Outputs      : -1 on failure or 0 on success
//
int32_t hdd_seek(int16_t fh, uint32_t loc) {

        // check if the file is openning
        if(fh != file.fHandler){
                printf("The file is not openning");
                return -1;
        }
        // change current position to loc
        if(loc <= file.blockSize){
                file.curPosition = loc;
                return 0;
        }
        // seeking out range of the file
        else{
                printf("seek out of range.\n");
                return -1;
        }
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : hddIOUnitTest
// Description  : Perform a test of the HDD IO implementation
//
// Inputs       : None
// Outputs      : 0 if successful or -1 if failure
//
// STUDENTS DO NOT MODIFY CODE BELOW UNLESS TOLD BY TA AND/OR INSTRUCTOR 
//
int hddIOUnitTest(void) {

	// Local variables
	uint8_t ch;
	int16_t fh, i;
	int32_t cio_utest_length, cio_utest_position, count, bytes, expected;
	char *cio_utest_buffer, *tbuf;
	HDD_UNIT_TEST_TYPE cmd;
	char lstr[1024];

	// Setup some operating buffers, zero out the mirrored file contents
	cio_utest_buffer = malloc(HDD_MAX_BLOCK_SIZE);
	tbuf = malloc(HDD_MAX_BLOCK_SIZE);
	memset(cio_utest_buffer, 0x0, HDD_MAX_BLOCK_SIZE);
	cio_utest_length = 0;
	cio_utest_position = 0;

	// Start by opening a file
	fh = hdd_open("temp_file.txt");
	if (fh == -1) {
		logMessage(LOG_ERROR_LEVEL, "HDD_IO_UNIT_TEST : Failure open operation.");
		return(-1);
	}

	// Now do a bunch of operations
	for (i=0; i<HDD_IO_UNIT_TEST_ITERATIONS; i++) {

		// Pick a random command
		if (cio_utest_length == 0) {
			cmd = HIO_UNIT_TEST_WRITE;
		} else {
			cmd = getRandomValue(HIO_UNIT_TEST_READ, HIO_UNIT_TEST_SEEK);
		}

		// Execute the command
		switch (cmd) {

		case HIO_UNIT_TEST_READ: // read a random set of data
			count = getRandomValue(0, cio_utest_length);
			logMessage(LOG_INFO_LEVEL, "HDD_IO_UNIT_TEST : read %d at position %d", bytes, cio_utest_position);
			bytes = hdd_read(fh, tbuf, count);
			if (bytes == -1) {
				logMessage(LOG_ERROR_LEVEL, "HDD_IO_UNIT_TEST : Read failure.");
				return(-1);
			}

			// Compare to what we expected
			if (cio_utest_position+count > cio_utest_length) {
				expected = cio_utest_length-cio_utest_position;
			} else {
				expected = count;
			}
			if (bytes != expected) {
				logMessage(LOG_ERROR_LEVEL, "HDD_IO_UNIT_TEST : short/long read of [%d!=%d]", bytes, expected);
				return(-1);
			}
			if ( (bytes > 0) && (memcmp(&cio_utest_buffer[cio_utest_position], tbuf, bytes)) ) {

				bufToString((unsigned char *)tbuf, bytes, (unsigned char *)lstr, 1024 );
				logMessage(LOG_INFO_LEVEL, "HIO_UTEST R: %s", lstr);
				bufToString((unsigned char *)&cio_utest_buffer[cio_utest_position], bytes, (unsigned char *)lstr, 1024 );
				logMessage(LOG_INFO_LEVEL, "HIO_UTEST U: %s", lstr);

				logMessage(LOG_ERROR_LEVEL, "HDD_IO_UNIT_TEST : read data mismatch (%d)", bytes);
				return(-1);
			}
			logMessage(LOG_INFO_LEVEL, "HDD_IO_UNIT_TEST : read %d match", bytes);


			// update the position pointer
			cio_utest_position += bytes;
			break;

		case HIO_UNIT_TEST_APPEND: // Append data onto the end of the file
			// Create random block, check to make sure that the write is not too large
			ch = getRandomValue(0, 0xff);
			count =  getRandomValue(1, HIO_UNIT_TEST_MAX_WRITE_SIZE);
			if (cio_utest_length+count >= HDD_MAX_BLOCK_SIZE) {

				// Log, seek to end of file, create random value
				logMessage(LOG_INFO_LEVEL, "HDD_IO_UNIT_TEST : append of %d bytes [%x]", count, ch);
				logMessage(LOG_INFO_LEVEL, "HDD_IO_UNIT_TEST : seek to position %d", cio_utest_length);
				if (hdd_seek(fh, cio_utest_length)) {
					logMessage(LOG_ERROR_LEVEL, "HDD_IO_UNIT_TEST : seek failed [%d].", cio_utest_length);
					return(-1);
				}
				cio_utest_position = cio_utest_length;
				memset(&cio_utest_buffer[cio_utest_position], ch, count);

				// Now write
				bytes = hdd_write(fh, &cio_utest_buffer[cio_utest_position], count);
				if (bytes != count) {
					logMessage(LOG_ERROR_LEVEL, "HDD_IO_UNIT_TEST : append failed [%d].", count);
					return(-1);
				}
				cio_utest_length = cio_utest_position += bytes;
			}
			break;

		case HIO_UNIT_TEST_WRITE: // Write random block to the file
			ch = getRandomValue(0, 0xff);
			count =  getRandomValue(1, HIO_UNIT_TEST_MAX_WRITE_SIZE);
			// Check to make sure that the write is not too large
			if (cio_utest_length+count < HDD_MAX_BLOCK_SIZE) {
				// Log the write, perform it
				logMessage(LOG_INFO_LEVEL, "HDD_IO_UNIT_TEST : write of %d bytes [%x]", count, ch);
				memset(&cio_utest_buffer[cio_utest_position], ch, count);
				bytes = hdd_write(fh, &cio_utest_buffer[cio_utest_position], count);
				if (bytes!=count) {
					logMessage(LOG_ERROR_LEVEL, "HDD_IO_UNIT_TEST : write failed [%d].", count);
					return(-1);
				}
				cio_utest_position += bytes;
				if (cio_utest_position > cio_utest_length) {
					cio_utest_length = cio_utest_position;
				}
			}
			break;

		case HIO_UNIT_TEST_SEEK:
			count = getRandomValue(0, cio_utest_length);
			logMessage(LOG_INFO_LEVEL, "HDD_IO_UNIT_TEST : seek to position %d", count);
			if (hdd_seek(fh, count)) {
				logMessage(LOG_ERROR_LEVEL, "HDD_IO_UNIT_TEST : seek failed [%d].", count);
				return(-1);
			}
			cio_utest_position = count;
			break;

		default: // This should never happen
			CMPSC_ASSERT0(0, "HDD_IO_UNIT_TEST : illegal test command.");
			break;

		}
	}

	// Close the files and cleanup buffers, assert on failure
	if (hdd_close(fh)) {
		logMessage(LOG_ERROR_LEVEL, "HDD_IO_UNIT_TEST : Failure read comparison block.", fh);
		return(-1);
	}
	free(cio_utest_buffer);
	free(tbuf);

	// Return successfully
	return(0);
}
































