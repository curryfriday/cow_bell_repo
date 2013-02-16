/*
 *	spi.c
 *
 *	Author: Greg Alexander
 *
 *	Created: 	Jan. 8, 2013
 *	Modified: 	Jan. 8, 2013
 *
 *	This code allows bytes to be sent and received over spi. It was initially designed to
 *	communicate with a MicroSD card.
 */

#include "spi.h"

void init_spi()
{
	P1SEL1 |= BIT5;	//UCA0CLK Function
	P2SEL1 |= BIT0 + BIT1; //UCA0SOMI, UCA0SIMO Function

	UCA0CTLW0 |= UCSWRST; //Not sure if this is necessary : FTO

	UCA0CTLW0 |= 		UCCKPH //capture data on first clock edge, changed on following
					//+ 	UCCKPL //inactive state is high
					+	UCMSB //MSB first
					+	UCMST //Master Mode
					+	UCSYNC //Synchronous mode
					+	UCSSEL_2 //SMCLK
					+	UCMODE_0 //3 pin mode
					;			//End of UCACTLW0

	UCA0BR0	|= 0x02;  //Prescalar = 2
	UCA0BR1	|= 0x00;

	UCA0CTLW0 &= ~UCSWRST; //disable reset

}//End of init_spi

unsigned char send_byte(unsigned char send_this)
{

	while(!(UCA0IFG & UCTXIFG)){}	//wait for SPI module to finish any current tasks

	UCA0TXBUF = send_this;

	while(UCA0STATW & UCBUSY){}	//wait for SPI module to finish transmitting

	return 0x01; //return success

}//End of send_byte


unsigned char rec_byte()
{
	unsigned char dummy;

	dummy = UCA0RXBUF; //empty receive buffer

	while(!(UCA0IFG & UCTXIFG)){}	//wait for SPI module to finish any current tasks

	UCA0TXBUF = 0xFF; //send dummy

	while(!(UCA0IFG & UCRXIFG)){}	//wait for SPI module to finish transmitting

	return UCA0RXBUF; //return received byte

}//End of rec_byte


