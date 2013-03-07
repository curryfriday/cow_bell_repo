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
void show_me(FRESULT);
void init_clocks();
void init_ports();
void init_i2c();


void get_temp(unsigned char*);
void make_ascii(unsigned char*, char *);

int main()
{
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	unsigned char temp[2];
	char text[10];
	unsigned char to_write[60];
	unsigned char start[] = {'s', 't', 'a', 'r', 't'};
	unsigned char first = 1;
	char store[100];
	unsigned char cnt = 0;

	FATFS fs;
	FRESULT res;
	WORD* bytes_written;

	//init_clocks();
    init_ports();
    init_i2c();

   /* int i = 0;
    while(i < 60)
    {
    	get_temp(temp);
    	make_ascii(temp, text);
    	to_write[i++] = text[0];
    	to_write[i++] = text[1];
    	to_write[i++] = text[2];
    	to_write[i++] = '\n';
    }*/

    pf_mount(&fs);
    pf_open("NEW.TXT");
    pf_write(start, 4, bytes_written);
    pf_write(start, 0, bytes_written);
    pf_mount(0);

   	while(1){



   	}


}

void show_me(FRESULT res)
{
	unsigned char test = (unsigned char) res;

	P1OUT &= 0xF8;
	P1OUT |= test;

}


void init_ports()
{
	P1DIR = BIT0 + BIT1 + BIT2;
	P1OUT = BIT0 + BIT1 + BIT2;

}//End of init_ports

void init_clocks()
{
	CSCTL0 = CSKEY; //set key

	CSCTL1 = DCOFSEL_0; //DC0 = 1MHZ

	CSCTL2 = SELS_3; //SMCLK = DC0

	CSCTL4 = LFXTOFF + HFXTOFF;

	CSCTL0_H = 0x01; //Lock clocks -- Not sure if this is necessary

}//End of init_clocks

void init_i2c()
{
	P1SEL1 |= BIT6 + BIT7;

	UCB0CTLW0 |= UCSWRST;

	UCB0CTLW0 =   UCMST		//master mode
				+ UCSYNC 	//synchronous
				+ UCSSEL_3	//SMCLK
				+ UCMODE_3	//i2c mode
				;

	UCB0BRW = 0x0008;

	UCB0I2CSA = 0x48; //Slave address for the TM102

	UCB0CTLW0 &= ~UCSWRST;

}//End of init_i2c

void get_temp(unsigned char* rec_byte)
{

	UCB0CTLW0 |= UCTR;
	UCB0CTLW0 |= UCTXSTT;
	while(UCB0CTLW0 & UCTXSTT){} //spin until complete address has been sent

	UCB0CTLW0 |= UCTXSTP;
	UCB0TXBUF = 0x00;
	while(UCB0STATW & UCBBUSY){}

	UCB0I2CSA = 0x48; //Slave address for the TM102
	UCB0CTLW0 &= ~UCTR;
	UCB0CTLW0 |= UCTXSTT;
	while(UCB0CTLW0 & UCTXSTT){} //spin until complete address has been sent

	while(!(UCB0IFG & UCRXIFG)){}//spin until byte is received
	rec_byte[0] = UCB0RXBUF;
	UCB0CTLW0 |= UCTXSTP;

	while(!(UCB0IFG & UCRXIFG)){}//spin until byte is received
	rec_byte[1] = UCB0RXBUF;

}//End of get_temp

void make_ascii(unsigned char * temp, char * text)
{
	text[0] = 48 + temp[0] / 100; 	//Convert hundreds digit to ascii
	text[1] = 48 + temp[0] / 10; 	//Convert tens digit to ascii
	text[2] = 48 + temp[0] % 10; 	//Convert ones digit to ascii

}//End of make_ascii

