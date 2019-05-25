
/* ----------------------------------------------------------------------------------------------------*/
/*
	mod_filespisd.h: gps logger module for SPI based SD card access.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#ifndef MOD_FILESPISD_H_
#define MOD_FILESPISD_H_

/* ----------------------------------------------------------------------------------------------------*/

#define CARD_SECTOR_SIZE 512

/* ----------------------------------------------------------------------------------------------------*/

boolean card_init (void);
boolean card_term (void);

boolean card_detect (void);
boolean card_enable (void);
boolean card_config_blocklength (void);

boolean card_state_suspend (void);
boolean card_state_resume (void);

unsigned int card_sector_read (const unsigned long sector, /*@out@*/ /*@notnull@*/ unsigned char * const buffer);
unsigned int card_sector_write (const unsigned long sector, /*@notnull@*/ const unsigned char * const buffer);

#ifdef TEST_ENABLED
test_result_t card_test (void);
#endif

/* ----------------------------------------------------------------------------------------------------*/

#endif /*MOD_FILESPISD_H_*/

