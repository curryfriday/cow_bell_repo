/*
 * Author: Greg Alexander
 * Created: January 8, 2013
 * Modified: January 24, 2013
 *
 * This code was modified from the skeleton code provided by ChaN for
 * PetitFS. It provides diskio functionality for the MSP430FR5739
 *
 * This code is HEAVILY based on the examples mades available on the Petit FatFs
 * webpage. The authors of this code are very grateful for the excellent examples
 * provided by ChaN.
 *
 * Low level disk I/O module skeleton for Petit FatFs (C)ChaN, 2009
 *
 */

#include "spi.h"
#include "diskio.h"
#include "spi_command_set.h"

//This function assumes the SD card has been powered for at least a second before being called
DSTATUS disk_initialize (void)
{
	unsigned i;

	init_spi();
	init_card_select_pin();

	//pull CS high, then send at least 74 clock pulses to the module with SIMO high
	deselect_card();
	for(i = 0; i <= 10; i++){
		rec_byte();
	}

	//pull CS low and send CMD0 to reset card and put it in SPI mode
	select_card();
	if (send_cmd(CMD_0, 0x00000000, 0x95) != 1){
		deselect_card();
		return RES_ERROR;
	}

	if(send_cmd(CMD_8, 0x000001AA, 0x87) == 1){ //Check voltage range on card. If a non-one value is returned, card is not sd v.2

		if(get_r7_resp(0xAA) != RES_OK){ //returns 1 if card works at 2.7-3.3V and check_pattern is valid
			deselect_card();
			return RES_ERROR;
		}

		if(check_ocr_voltage_range() != RES_OK){
			deselect_card();
			return RES_ERROR;
		}

		if(wait_for_card_to_finish_init() != RES_OK){
			deselect_card();
			return RES_ERROR;
		}

		if(check_card_type() != RES_OK){
			deselect_card();
			return RES_ERROR;
		}

	}//close if cmd_8
	//else{



	//}//Version 1.0 or earlier

	deselect_card();
	return RES_OK;
}

DRESULT disk_readp (
	BYTE* dest,			/* Pointer to the destination object */
	DWORD sector,		/* Sector number (LBA) */
	WORD sofs,			/* Offset in the sector */
	WORD count			/* Byte count (bit15:destination) */
)
{
	DRESULT res;
	USHORT timeout;
	USHORT bytes_to_read;

	select_card();

	if(!(card_type & CT_BLOCK)) sector *= 512;

	res = RES_ERROR;

	if(send_cmd(CMD_17, sector, 0x00) != 0){
		deselect_card();
		return RES_ERROR;
	}

	timeout = 4000;

	while((rec_byte() != 0xFE) && timeout--){}

	if(timeout){
		bytes_to_read = 514 - sofs - count;

		//Pass through offset
		if(sofs){
			do{
				rec_byte();
			}while(--sofs); //Debug: check this
		}
		//get requested data bytes
		if(dest){
			do {
					*dest++ = rec_byte();
			} while (--count);
		}else{
			do {
				rec_byte();//DEBUG: Not sure if i need this. Examples uses	FORWARD(rec_byte()); for communication with a computer
			}while(--count);
		}

		//skip over trailing bytes
		do {
			rec_byte();
		}while(--bytes_to_read);

		res = RES_OK;

	}//close if timeout

	deselect_card();
	rec_byte();

	return res;
}

DRESULT disk_writep (
	const BYTE* buff,		/* Pointer to the data to be written, NULL:Initiate/Finalize write operation */
	DWORD sc				/* Sector number (LBA) or Number of bytes to send */
)
{
	DRESULT res;
	WORD byte_count;
	static WORD bytes_left_in_block; //counter used to write data

	select_card();

	res = RES_ERROR;

	if (buff) {

		byte_count = (WORD)sc;

		while(bytes_left_in_block && byte_count){
			send_byte(*buff++);
			byte_count--;
			bytes_left_in_block--;
		}//end of while

		res = RES_OK;

	}//end of if(buf)
	else if (sc) {

		if(!(card_type & CT_BLOCK))
			sc *= 512; //multiply address by 512 if the card is byte addrssed

		if(send_cmd(WRITE_BLOCK, sc, 0x00) == 0){
			send_byte(0xFF); //Send header byte 1
			send_byte(0xFE); //Send header byte 2
			bytes_left_in_block = 512;
		}

		res = RES_OK;

	}//end of 1st else
	else{

		fill_zeros(bytes_left_in_block);
		res = check_write_success();

		deselect_card();
	}//end 2nd else

	return res;

}//End of writep


BYTE send_cmd(BYTE cmd, DWORD arg, BYTE crc)
{
	unsigned char i;
	BYTE received;

	union {
		BYTE	sep_arg[4];
		DWORD	full_arg;
	} separate;

	//Check for app command, if so, send APP_CMD first
	if(cmd == ACMD_41 || cmd == SET_WR_BLOCK_ERASE_COUNT){
		send_cmd(APP_CMD, 0x00, 0x00); //little bit o' recursion, may take this out later
	}

	separate.full_arg = arg; //Divide 32-bit arg into 4 "separated args" - see union above

	send_byte(cmd | CMD); //send byte with the command flag (0x40)

	send_byte(separate.sep_arg[3]); //send each of the 4-bytes stored in the sep_arg array
	send_byte(separate.sep_arg[2]);
	send_byte(separate.sep_arg[1]);
	send_byte(separate.sep_arg[0]);

	send_byte(crc | 0x01); //last byte in the command is the CRC with end bit

	//The following loop waits for a response from the card
	//it times out after 10 cycles
	for(i = 0; i < 10; i++){
		received = rec_byte();
		if(!(received & 0x80)) //break on r1 response
			break;
	}

	return received;

}//End send_cmd

inline void fill_zeros(WORD bytes_left)
{
	USHORT zero_count;

	zero_count = bytes_left + 2; //tofix: figure out why the +2

	while(zero_count){
		zero_count--;
		send_byte(0x20);
	}

}//End of fill_zeros

BYTE get_r7_resp(BYTE check_pattern)
{
	BYTE r7_resp[4];
	BYTE i;

	for(i = 0; i < 4; i++){
		r7_resp[i] = rec_byte();
	}


	if ( (r7_resp[2] == 0x01) && (r7_resp[3] == check_pattern) ){//true if it operates at appropriate voltages and if check pattern is valid
//		send_byte(r7_resp[2]);
	//	send_byte(r7_resp[3]);

		return RES_OK;
	}
	else
		return RES_ERROR;

}//End of get_r7_resp()

BYTE check_ocr_voltage_range()
{
	BYTE ocr_vals[4];

	if(send_cmd(CMD_58, 0x00000000, 0x00) == 1){ //read OCR to check voltage range
		read_ocr(ocr_vals);

		if(ocr_vals[1] & 0x30)//check if card can support 3.2-3.3 or 3.3-3.4 V supply (implementation dependent)
				return RES_OK;
	}//close cmd_58 for voltage range

	return RES_ERROR;

}//End of check_ocr_voltage_range

BYTE wait_for_card_to_finish_init()
{
	BYTE temp;
	BYTE i;

	for(i = 0; i < 10; i++){ //i defines timeout period, successful communication will exit the loop via a break
		temp = send_cmd(ACMD_41, 0x40000000, 0x00);
		if(temp == 0)
			break;
	}//end of for

	if(temp == 0)//done initializing
		return RES_OK;
	else
		return RES_ERROR;

}//End of wait_for_card_to_finish_init

BYTE check_card_type()
{
	BYTE ocr_vals[4];

	if(send_cmd(CMD_58, 0x00000000, 0x00) == 0){	//read OCR to check card type
		read_ocr(ocr_vals);

		if(ocr_vals[0] & 0x40){ // true if card is sdhc or sdxc
			card_type = CT_SD2 | CT_BLOCK; //sdhc and sdxc cards automatically have 512 byte blocks
			return RES_OK;
		}
		else if(send_cmd(CMD_16, 0x00000200, 0x00) == 0x00){ //sdsc, so we need to make sure the block length is 512
			card_type = CT_SD2 | CT_BLOCK;
			return RES_OK;
		}//close cmd_16
		else
			return RES_ERROR;
	}//close cmd_58 for check card type

	return RES_ERROR;

}//End of check_card_type

DRESULT check_write_success()
{
	USHORT timeout = 5000;

	if((rec_byte() & 0x1F) == 0x05){
		while(timeout){
			timeout--;
			if(rec_byte() == 0xFF)
				//Greg added night before midterm
				if(send_status() == RES_OK)
					return RES_OK;
				else
					return RES_ERROR;
		}
	}



	return RES_ERROR;

}//End of check_write_success

DRESULT send_status()
{
	BYTE r2_resp[2];

	send_byte(CMD_13 | CMD); //transmit CMD_13 (=SEND_STATUS command)
	send_byte(0x00);//stuff byte 1
	send_byte(0x00);//stuff byte 2
	send_byte(0x00);//stuff byte 3
	send_byte(0x00);//stuff byte 4
	send_byte(0x00);//stuff byte 5 -- entire command is now sent

	rec_byte(); //burn a byte... I didn't think we would need this, but apparently we do

	//Get R2 response
	r2_resp[0] = rec_byte(); //first byte is same as R1 response
	r2_resp[1] = rec_byte(); //second byte is where the money be


	//Write was successful if no bits are set in the second byte of R2
	if(r2_resp[1] == 0x00)
			return RES_OK;
	else
			return RES_ERROR;

}

void read_ocr(BYTE* ocr_vals)
{
	ocr_vals[0] = rec_byte();
	ocr_vals[1] = rec_byte();
	ocr_vals[2] = rec_byte();
	ocr_vals[3] = rec_byte();
}

inline void init_card_select_pin()
{
	CS_PDIR |= CS_BIT; //output
	CS_PREN |= CS_BIT; //turn on resistors
	CS_POUT |= CS_BIT; //pullup
}//End of init_slave_select_pin

inline void select_card()
{
	CS_POUT &= ~CS_BIT;
}//End of select_card

inline void deselect_card()
{
	CS_POUT |= CS_BIT;
}//End of deselect_card

