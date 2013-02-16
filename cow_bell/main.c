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

#include <msp430fr5739.h>
#include "pff.h"
#include "spi.h"
#include "integer.h"
#include "diskio.h"

void init_ports();
unsigned char debounce_pin(unsigned char);

int main()
{
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	unsigned char to_write[20] = "I added something.,";
	unsigned char read_back[50];
	int i;

	FATFS* fs;
	FRESULT res;
	WORD* bytes_written;

	init_spi();
    init_ports();


    while(1){

    	if(debounce_pin(0)){
    		//disk_initialize();
    		res = pf_mount(fs);
    		res = pf_open("NEW.TXT");
    		res = pf_write(to_write, 20, bytes_written);
    		res = pf_mount(0);
    	}

   	}


}

void init_ports()
{
	P1DIR |= 0x01;
	P1OUT &= ~(0x01);

	PJDIR |= 0x0F;
	PJOUT = 0x09;

	P3DIR |= 0xF0;
	P3OUT = 0x00;

	P4DIR &= ~BIT0; //P4.0 is input
	P4REN |= BIT0; //Enable resistor
	P4OUT |= BIT0; //Pullup

}//End of init_ports

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
