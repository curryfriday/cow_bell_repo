/*
 *	main.c
 *
 *	Author: Greg Alexander
 *
 *	Created: 	Jan. 8, 2013
 *	Modified: 	Feb. 12, 2013
 *
 *	This code is in place to test the functions written for the sd_pff library
 */

#include <msp430fr5969.h>
#include "pff.h"
#include "spi.h"
#include "integer.h"
#include "diskio.h"
#include "spi.h"

#ifndef DEBUG
#define DEBUG 1
#define LIGHT_OFF P1OUT &= ~BIT1;
#define BLINK1 while(1){__delay_cycles(1000000);P1OUT^=BIT1;}
#define BLINK2 while(1){__delay_cycles(500000);P1OUT^=BIT1;}
#define BLINK3 while(1){__delay_cycles(200000);P1OUT^=BIT1;}
#define BLINK4 while(1){__delay_cycles(50000);P1OUT^=BIT1;}
#endif

void init_debug();
void check_mount(FRESULT);
void check_open(FRESULT);
void init_ports();
void show_me(FRESULT);

unsigned char debounce_pin(unsigned char);

int main()
{
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	unsigned char to_write[58] = "Chris owes me a beer. And the FATFS API is really dumb.";
	unsigned char read_back[50];
	unsigned char i;

	FATFS fs;
	FRESULT res;
	WORD* bytes_written;

    init_ports();

	#if DEBUG
   		init_debug();
	#endif

   	res = pf_mount(&fs);
 	res = pf_open("NEW.TXT");

  //	check_open(res);

   	res = pf_write(to_write, 58, bytes_written);

   	res = pf_write(to_write, 0, bytes_written);
	show_me(res);
   	//res = pf_mount(0);

   	while(1){
   	}


}

void init_ports()
{
}//End of init_ports

void init_debug()
{
	P1DIR |= BIT0 + BIT1 + BIT2;
	P1OUT  |= BIT0 + BIT1 + BIT2;
}

void show_me(FRESULT res)
{
	unsigned char test = (unsigned char) res;

	P1OUT &= 0xF8;
	P1OUT |= test;

}


void check_mount(FRESULT res)
{
	switch(res){
		case FR_OK: 			BLINK3;
								break;
		case FR_NOT_READY:		BLINK1;
								break;
		case FR_DISK_ERR:		BLINK2;
								break;
		case FR_NO_FILESYSTEM:	LIGHT_OFF;
								break;
		default:				BLINK4;
								break;
	}

}

void check_open(FRESULT res)
{
		switch(res){
			case FR_OK: 			BLINK4;
									break;
			case FR_NO_FILE:		BLINK1;
									break;
			case FR_NO_PATH:		BLINK2;
									break;
			case FR_DISK_ERR:		BLINK3;
									break;
			case FR_NOT_ENABLED:	LIGHT_OFF;
									break;
		}

}


//The following debounce routine is based on the
//digital filter w/ Schmitt Trigger in the class
//slides. The general form is the same, but I made several
//changes. It works with an entire port, rather than just one pin.
//The pin currently being tested is specified in the pin arg.
unsigned char debounce_pin(unsigned char pin)
{

	//Indicies 0-7 for old_val and flag represent pb's 0-7 on
	//a given port. For this implementation, it is PORTC. The
	//port can be changed in the "if(bit_is_clear(PINx...))"
	//statement a few lines down
	static int old_val[] = {0, 0, 0, 0, 0, 0, 0, 0};
	static int flag[] = {0, 0, 0, 0, 0, 0, 0, 0};

	//Cool little division by subtraction routine from the slides
	//Explained:
	//	old_val >> 2 = old_val/4
	//	old_val - old_val/4 = .75*old_val
	old_val[pin] = old_val[pin] - (old_val[pin] >> 2);

	//Check bit: if button is pressed, add 0x4F to old_val
	//Could change this number based on the switch being
	//debounced
	if(!(P4IN & (1 << pin)))
		old_val[pin] = old_val[pin] + 0x3F;

	//Implement the software-based Schmitt Trigger
	//with high and low threshold values of 0xF0 and 0x0F.
	//Again, these can be changed based on preference/switch
	//being tested.
	if((old_val[pin] > 0xF0) && (flag[pin] == 0)){
		flag[pin] = 1;
	}
	//Upon dropping below the low trigger threshold,
	//the function returns 1. In all other cases,
	//it returns 0.
	if((old_val[pin] < 0x0F) && (flag[pin] == 1)){
		flag[pin] = 0;
		return 1;
	}

	//The return value of this function (output[pin]) tells
	//the calling function whether or not the button at pin
	//is 'pressed'.
	return 0;

}//End of debounce_pin
