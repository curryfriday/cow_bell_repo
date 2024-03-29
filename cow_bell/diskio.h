/*-----------------------------------------------------------------------
/  PFF - Low level disk interface modlue include file    (C)ChaN, 2009
/-----------------------------------------------------------------------*/

#ifndef _DISKIO

#include "integer.h"

/* Status of Disk Functions */
typedef BYTE	DSTATUS;

/* Results of Disk Functions */
typedef enum {
	RES_OK = 0,		/* 0: Function succeeded */
	RES_ERROR,		/* 1: Disk error */
	RES_NOTRDY,		/* 2: Not ready */
	RES_PARERR		/* 3: Invalid parameter */
} DRESULT;

/*---------------------------------------*/
/* Prototypes for disk control functions */

DSTATUS disk_initialize (void);
DRESULT disk_readp (BYTE*, DWORD, WORD, WORD);
DRESULT disk_writep (const BYTE*, DWORD);

BYTE send_cmd(BYTE cmd, DWORD arg, BYTE crc);

inline void select_card();
inline void deselect_card();
inline void init_card_select_pin();

//Init support functions
BYTE get_r7_resp(BYTE);
BYTE check_ocr_voltage_range();
BYTE wait_for_card_to_finish_init();
BYTE check_card_type();
void read_ocr(BYTE*);
//end of init support functions

//	writep support functions
inline void fill_zeros(WORD);
DRESULT check_write_success();
DRESULT send_status();
//end of writep support functions

static BYTE card_type;

#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */

/* Card type flags (CardType) */
#define CT_MMC				0x01	/* MMC ver 3 */
#define CT_SD1				0x02	/* SD ver 1 */
#define CT_SD2				0x04	/* SD ver 2 */
#define CT_SDC				(CT_SD1|CT_SD2)	/* SD */
#define CT_BLOCK			0x08	/* Block addressing */

//Added by Greg
#define CS_POUT		P1OUT //Slave select port
#define CS_PDIR		P1DIR //Slave port direction
#define CS_PREN		P1REN //Slave resistor enable
#define CS_BIT		BIT3  //Slave select pin



#define _DISKIO
#endif
