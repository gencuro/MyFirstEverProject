#ifndef __VARIABLE_H
#define __VARIABLE_H

#define MEMID_MAX_SIZE 20
#define PATHFILE_MAX_SIZE 200

//Com port default settings
#define MAX_RS232 4

#define DEFAULT_BAUDRATE 115200
#define DEFAULT_NBIT 8
#define DEFAULT_SBIT 0
#define DEFAULT_PARITY 0 //no parity
#define DEFAULT_ADDR 0
#define DEFAULT_LLONG 0
#define TIME_OUT -32000

//operations
#define OP_READ   1
#define OP_WRITE  2
#define OP_VERIFY 3

//memtypes
#define K4M  1
#define K128 2	

#define WORK_SIZE 0x80

//K128
#define K128_MAX_SIZE 0x20000//0x1FFFF
#define K128_SECTOR_SIZE 0x4000

//K4M
#define K4M_MAX_SIZE 0x1000//0xFFF

//errors
#define NO_ERR 0 
#define OPEN_COM_ERR 1 
#define GETCOMSTAT_ERR 2 
#define SETCOMSTAT_ERR 3 
#define COM_CLOSE_ERR 4 
#define KZU_CONECTED_ERR 5 
#define COMMAND_ERR 6 
#define OPEN_CREAT_FILE_ERR 7 
#define WRITE_FILE_ERR 8 
#define READ_FILE_ERR 9 
#define ERASE_SECTOR_ERR 10 
#define WRITE_KZU_ERR 11 
#define VERIFY_ERR 12 
#define WORK_SIZE_ERR 13 

#define RS232_INVALID_PARAM	14
#define TIME_OUT_ERR 15


#endif
