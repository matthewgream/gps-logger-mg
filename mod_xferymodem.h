
/* ----------------------------------------------------------------------------------------------------*/
/*
	mod_xferymodem.h: gps logger module for transfer with ymodem.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#ifndef MOD_XFERYMODEM_H_
#define MOD_XFERYMODEM_H_

/* ----------------------------------------------------------------------------------------------------*/

typedef unsigned char ym_error_t;

#define YM_ERR_NONE				((ym_error_t) 0x00)
#define YM_ERR_TIMEOUT			((ym_error_t) 0x01)
#define YM_ERR_RETRIESEXCEEDED	((ym_error_t) 0x02)
#define YM_ERR_PEERTERMINATED	((ym_error_t) 0x03)
#define YM_ERR_TRYAGAIN			((ym_error_t) 0x04)
#define YM_ERR_UNSUPPORTED		((ym_error_t) 0x05)

typedef boolean (*ym_io_read_char_t) (/*@notnull@*/ /*@out@*/ unsigned char * const data, unsigned int timeout);
typedef boolean (*ym_io_write_char_t) (unsigned char data);
typedef boolean (*ym_io_flush_t) (unsigned int timeout);

typedef struct
{
		/*@shared@*/ ym_io_read_char_t io_read;
		/*@shared@*/ ym_io_write_char_t io_write;
		/*@shared@*/ ym_io_flush_t io_flush;
}
ymodem_config_t;

/* ----------------------------------------------------------------------------------------------------*/

ym_error_t ymodem_init (/*@notnull@*/ const ymodem_config_t * const config);
ym_error_t ymodem_term (void);

ym_error_t ymodem_tx_start (void);
ym_error_t ymodem_tx_sendfile_head (/*@notnull@*/ const char * const filename, unsigned long filesize);
ym_error_t ymodem_tx_sendfile_data (/*@notnull@*/ const unsigned char * const datapntr, unsigned int datasize);
ym_error_t ymodem_tx_sendfile_tail (void);
ym_error_t ymodem_tx_finish (void);

#ifdef TEST_ENABLED
test_result_t ymodem_test (void);
#endif

/* ----------------------------------------------------------------------------------------------------*/

#endif /*MOD_XFERYMODEM_H_*/

