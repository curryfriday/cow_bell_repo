/*
 *	spi.h
 *
 *	Author: Greg Alexander
 *
 *	Created: 	Jan. 8, 2013
 *	Modified: 	Jan. 8, 2013
 *
 * Header file
 * This code allows bytes to be sent and received over spi. It was initially designed to
 * communicate with a MicroSD card.
 */

#ifndef SPI_H_
#define SPI_H_

#include "msp430fr5739.h"

void init_spi();
unsigned char send_byte(unsigned char);
unsigned char rec_byte();


#endif /* SPI_H_ */
