/*-----------------------------------------------------------------------*/
/*
 * Author: Greg Alexander
 * Created: January 9, 2013
 * Modified: January 9, 2013
 *
 * This code contains macros for the SPI command set for interfacing
 * with an SD card.
/*-----------------------------------------------------------------------*/

#ifndef SPI_COMMAND_SET_H_
#define SPI_COMMAND_SET_H_

#define CMD							0x40 //Specifies a command: must be or'd with any of the commands below

#define CMD_0						0
#define SEND_OP_COND				0x01
#define ACMD_41						41 //ACMD: must follow APP_CMD
#define CMD_8						8
#define SEND_CSD					0x09
#define SEND_CID					0x0A
#define STOP_TRANSMISSION			0x0C
#define CMD_13						13 //Send status
#define CMD_16						16
#define CMD_17						17
#define READ_MULTIPLE_BLOCK			0x12
#define SET_BLOCK_COUNT				0x17
#define SET_WR_BLOCK_ERASE_COUNT	0x17 //ACMD: must follow APP_CMD
#define WRITE_BLOCK					0x18
#define WRITE_MULTIPLE_BLOCK		0x19
#define APP_CMD						0x37
#define CMD_58						58

#endif /* SPI_COMMAND_SET_H_ */
