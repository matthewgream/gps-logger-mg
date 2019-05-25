
/* ----------------------------------------------------------------------------------------------------*/
/*
	mod_xferymodem.c: gps logger module for transfer with ymodem.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#include "common.h"
#include "mod_util.h"
#include "mod_xferymodem.h"

/* ----------------------------------------------------------------------------------------------------*/

typedef struct
{
		boolean flg_usemodeg;
		unsigned int blk_num;
		unsigned int cnt_can;

		/*@shared@*/ ym_io_read_char_t io_read;
		/*@shared@*/ ym_io_write_char_t io_write;
		/*@shared@*/ ym_io_flush_t io_flush;
}
ymodem_t;

static ymodem_t ymodem_instance;

/* ----------------------------------------------------------------------------------------------------*/

/*
 * XMODEM/YMODEM PROTOCOL REFERENCE (Chuck Forsberg, 1988)
 * http://www.techfest.com/hardware/modem/xymodem.htm
 * This version only supports CRC16, not CHECKSUM
 */

#define YM_CTL_SOH				((unsigned char) 0x01)
#define YM_CTL_STX				((unsigned char) 0x02)
#define YM_CTL_EOT				((unsigned char) 0x04)
#define YM_CTL_ACK				((unsigned char) 0x06)
#define YM_CTL_NAK				((unsigned char) 0x15)
#define YM_CTL_CAN				((unsigned char) 0x18)
#define YM_CTL_EOB				((unsigned char) 0x1A)
#define YM_CTL_CHG				((unsigned char) 'G')
#define YM_CTL_CHC				((unsigned char) 'C')

#define YM_TMO_NONE				(0*1000)
#define YM_TMO_STEP				(10*1000)
#define YM_TMO_INIT				(50*1000)
#define YM_TMO_WACK				(5*1000)
#define YM_TMO_BLCK				(2*1000)
#define YM_TMO_SEND				(10*1000)
#define YM_TMO_FLSH_INIT		(0*1000)

#define YM_BLK_SIZE_128BYTES	(128)
#define YM_BLK_SIZE_1KBYTES		(1024)

#define YM_CFG_RETRYCOUNT		(10)
#define YM_CFG_CANCELCOUNT		(5)

/* ----------------------------------------------------------------------------------------------------*/

/*@+ignoresigns@*/ /*@+ignorequals@*/
static const unsigned short ym_crc16_tab [256] =
{
		0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
		0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
		0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
		0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
		0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
		0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
		0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
		0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
		0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
		0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
		0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
		0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
		0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
		0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
		0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
		0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
		0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
		0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
		0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
		0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
		0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
		0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
		0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
		0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
		0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
		0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
		0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
		0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
		0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
		0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
		0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
		0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};
/*@=ignoresigns@*/ /*@=ignorequals@*/

#define YM_CRC16_DFL		((unsigned char) 0x0000)
#define ym_crc16_upd(c,d)	ym_crc16_tab [((c) >> 8)] ^ (((c) << 8) ^ (unsigned short) ((d) & 0xFF))

/* ----------------------------------------------------------------------------------------------------*/

static ym_error_t ym_tx_init (/*@notnull@*/ ymodem_t * const instance)
{
		instance->flg_usemodeg = FALSE;
		instance->blk_num = 0;
		instance->cnt_can = 0;

		(void) instance->io_flush (YM_TMO_FLSH_INIT);

		return YM_ERR_NONE;
}

/* ----------------------------------------------------------------------------------------------------*/

static ym_error_t ym_tx_fail (/*@notnull@*/ ymodem_t * const instance, ym_error_t error)
{
		int cnt_cancels = YM_CFG_CANCELCOUNT;

		assert (instance != NULL);

		while (cnt_cancels > 0)
		{
				(void) instance->io_write (YM_CTL_CAN);

				cnt_cancels--;
		}

		return error;
}

/* ----------------------------------------------------------------------------------------------------*/

static ym_error_t ym_tx_recv_resp (/*@notnull@*/ ymodem_t * const instance, unsigned char ch)
{
		assert (instance != NULL);

		if (ch == YM_CTL_CAN)
		{
				if (instance->cnt_can++ > 0)
						return YM_ERR_PEERTERMINATED;
		}
		else if (instance->cnt_can > 0)
		{
				instance->cnt_can = 0;
		}

		if (ch == YM_CTL_ACK)
				return YM_ERR_NONE;

		return YM_ERR_TRYAGAIN;
}

/* ----------------------------------------------------------------------------------------------------*/

static ym_error_t ym_tx_recv_cmnd (/*@notnull@*/ ymodem_t * const instance)
{
		int tmo_command = YM_TMO_INIT;

		assert (instance != NULL);

		while (tmo_command > 0)
		{
				unsigned char ch;

				if (instance->io_read (&ch, (unsigned int) YM_TMO_STEP) == TRUE)
				{
						if (ch == YM_CTL_CHG)
						{
								instance->flg_usemodeg = TRUE;
								instance->cnt_can = 0;
								return YM_ERR_NONE;
						}
						else if (ch == YM_CTL_CHC)
						{
								instance->cnt_can = 0;
								return YM_ERR_NONE;
						}
						else if (ch == YM_CTL_NAK)
						{
								return YM_ERR_UNSUPPORTED;
						}
						else if (ch == YM_CTL_CAN)
						{
								if (instance->cnt_can++ > 0)
										return YM_ERR_PEERTERMINATED;
						}
				}

				tmo_command -= YM_TMO_STEP;
		}

		return YM_ERR_TIMEOUT;
}

/* ----------------------------------------------------------------------------------------------------*/

static ym_error_t ym_tx_send_term (/*@notnull@*/ ymodem_t * const instance)
{
		int cnt_retries = YM_CFG_RETRYCOUNT;

		assert (instance != NULL);

		while (cnt_retries > 0)
		{
				unsigned char ch;

				(void) instance->io_write (YM_CTL_EOT);

				if (instance->io_read (&ch, (unsigned int) YM_TMO_WACK) == TRUE)
				{
						ym_error_t r;

						if ((r = ym_tx_recv_resp (instance, ch)) != YM_ERR_TRYAGAIN)
								return r;
				}

				cnt_retries--;
		}

		return YM_ERR_RETRIESEXCEEDED;
}

/* ----------------------------------------------------------------------------------------------------*/

static ym_error_t ym_tx_send_block (/*@notnull@*/ ymodem_t * const instance, /*@notnull@*/ const unsigned char * const data, const unsigned int size)
{
		int cnt_retries = YM_CFG_RETRYCOUNT;

		assert (instance != NULL);

		while (cnt_retries > 0)
		{
				unsigned char ch;
				unsigned int i = 0, bsiz = (unsigned int) ((size <= (unsigned int) YM_BLK_SIZE_128BYTES) ? YM_BLK_SIZE_128BYTES : YM_BLK_SIZE_1KBYTES);
				unsigned short crc16 = YM_CRC16_DFL;

				(void) instance->io_write ((bsiz == (unsigned int) YM_BLK_SIZE_128BYTES) ? YM_CTL_SOH : YM_CTL_STX);
				(void) instance->io_write ((unsigned char) (instance->blk_num) & 0xFF);
				(void) instance->io_write ((unsigned char) (~instance->blk_num) & 0xFF);
				
				while (i < size) {
						(void) instance->io_write (data [i]);
						crc16 = ym_crc16_upd (crc16, data [i]); i++;
				}
				while (i < bsiz) {
						(void) instance->io_write (YM_CTL_EOB);
						crc16 = ym_crc16_upd (crc16, YM_CTL_EOB); i++;
				}
				crc16 = ym_crc16_upd (crc16, 0x00);
				crc16 = ym_crc16_upd (crc16, 0x00);

				(void) instance->io_write ((unsigned char) (crc16 >> 8) & 0xFF);
				(void) instance->io_write ((unsigned char) (crc16) & 0xFF);

				if (instance->flg_usemodeg == TRUE)
				{
						instance->blk_num++;

						if (instance->io_read (&ch, YM_TMO_NONE) == TRUE)
								return ym_tx_recv_resp (instance, ch); /* could be CAN */

						return YM_ERR_NONE;
				}
				else
				{
						if (instance->io_read (&ch, (unsigned int) YM_TMO_WACK) == TRUE)
						{
								ym_error_t r;

								if ((r = ym_tx_recv_resp (instance, ch)) != YM_ERR_TRYAGAIN)
								{
										instance->blk_num++;
										return r;
								}
						}
				}

				cnt_retries--;
		}

		return YM_ERR_RETRIESEXCEEDED;
}

/* ----------------------------------------------------------------------------------------------------*/

static void ym_tx_make_block_zero (/*@notnull@*/ ymodem_t * const instance, /*@out@*/ /*@notnull@*/ unsigned char * const datapntr, unsigned int datasize, /*@notnull@*/ const char * filename, unsigned long filesize)
{
		unsigned int dataoffs = 0;

		assert (instance != NULL);
		assert (datapntr != NULL);
		assert (filename != NULL);

		while (dataoffs < (datasize - 1) && *filename != '\0')
				datapntr [dataoffs++] = (unsigned char) *filename++;
		datapntr [dataoffs++] = '\0';
		if (dataoffs < (datasize - 8))
				dataoffs += util_stratol ((char *) &datapntr [dataoffs], (signed long) filesize);
		while (dataoffs < datasize)
				datapntr [dataoffs++] = '\0';
}

/* ----------------------------------------------------------------------------------------------------*/

static void ym_tx_make_block_null (/*@notnull@*/ ymodem_t * const instance, /*@out@*/ /*@notnull@*/ unsigned char * const datapntr, unsigned int datasize)
{
		unsigned int dataoffs = 0;

		assert (instance != NULL);
		assert (datapntr != NULL);

		while (dataoffs < datasize)
				datapntr [dataoffs++] = '\0';
}

/* ----------------------------------------------------------------------------------------------------*/

ym_error_t ymodem_init (const ymodem_config_t * const config)
{
		/*@shared@*/ ymodem_t * const instance = &ymodem_instance;

		assert (config != NULL);
		assert (config->io_read != NULL && config->io_write != NULL && config->io_flush != NULL);

		instance->io_read = config->io_read;
		instance->io_write = config->io_write;
		instance->io_flush = config->io_flush;

		return YM_ERR_NONE;
}

/* ----------------------------------------------------------------------------------------------------*/

ym_error_t ymodem_term (void)
{
		return YM_ERR_NONE;
}

/* ----------------------------------------------------------------------------------------------------*/

ym_error_t ymodem_tx_start (void)
{
		return YM_ERR_NONE;
}

/* ----------------------------------------------------------------------------------------------------*/

ym_error_t ymodem_tx_sendfile_head (/*@notnull@*/ const char * const filename, unsigned long filesize)
{
		/*@shared@*/ ymodem_t * const instance = &ymodem_instance;
		ym_error_t r;
		unsigned char databuff [YM_BLK_SIZE_128BYTES];

		assert (filename != NULL);

		(void) ym_tx_init (instance);

		if ((r = ym_tx_recv_cmnd (instance)) != YM_ERR_NONE)
				return ym_tx_fail (instance, r);

		ym_tx_make_block_zero (instance, databuff, (unsigned int) sizeof (databuff), filename, filesize);
		if ((r = ym_tx_send_block (instance, databuff, (unsigned int) sizeof (databuff))) != YM_ERR_NONE)
				return ym_tx_fail (instance, r);

		if ((r = ym_tx_recv_cmnd (instance)) != YM_ERR_NONE)
				return ym_tx_fail (instance, r);

		return YM_ERR_NONE;
}

ym_error_t ymodem_tx_sendfile_data (/*@notnull@*/ const unsigned char * const datapntr, unsigned int datasize)
{
		/*@shared@*/ ymodem_t * const instance = &ymodem_instance;
		ym_error_t r;

		assert (datapntr != NULL);
		assert (datasize <= (unsigned int) YM_BLK_SIZE_1KBYTES);

		if ((r = ym_tx_send_block (instance, datapntr, datasize)) != YM_ERR_NONE)
				return ym_tx_fail (instance, r);

		return YM_ERR_NONE;
}

ym_error_t ymodem_tx_sendfile_tail (void)
{
		/*@shared@*/ ymodem_t * const instance = &ymodem_instance;
		ym_error_t r;

		if ((r = ym_tx_send_term (instance)) != YM_ERR_NONE)
				return ym_tx_fail (instance, r);

		return YM_ERR_NONE;
}

/* ----------------------------------------------------------------------------------------------------*/

ym_error_t ymodem_tx_finish (void)
{
		/*@shared@*/ ymodem_t * const instance = &ymodem_instance;
		ym_error_t r;
		unsigned char databuff [YM_BLK_SIZE_128BYTES];

		(void) ym_tx_init (instance);

		if ((r = ym_tx_recv_cmnd (instance)) != YM_ERR_NONE)
				return ym_tx_fail (instance, r);

		ym_tx_make_block_null (instance, databuff, (unsigned int) sizeof (databuff));
		if ((r = ym_tx_send_block (instance, databuff, (unsigned int) sizeof (databuff))) != YM_ERR_NONE)
				return ym_tx_fail (instance, r);

		return YM_ERR_NONE;
}

/* ----------------------------------------------------------------------------------------------------*/

#ifdef TEST_ENABLED

typedef struct {
		unsigned char vector [10];
		int length;
		unsigned short crc16;
} ymodem_test_crc16_data_t;

/*@+charint@*/ /*@+ignoresigns@*/
static const ymodem_test_crc16_data_t ymodem_test_crc16_data [] = {
		{ { 0xAC, 0x1D, 0x82, 0xF5, 0x3D, 0x43, 0xAD, 0x00, 0x00, 0x00 }, 9, (unsigned short) 0xF830 },
		{ { 0x70, 0x6A, 0x77, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 3, (unsigned short) 0x3299 },
		{ { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' }, 10, (unsigned short) 0xD321 }
};
/*@=charint@*/ /*@=ignoresigns@*/

static boolean ymodem_test_crc16 (void)
{
		int i, j;
		for (i = 0; i < (int) (sizeof (ymodem_test_crc16_data) / sizeof (ymodem_test_crc16_data_t)); i++) {
				unsigned short crc16 = YM_CRC16_DFL;
				for (j = 0; j < ymodem_test_crc16_data [i].length; j++)
						crc16 = ym_crc16_upd (crc16, ymodem_test_crc16_data [i].vector [j]);
				crc16 = ym_crc16_upd (crc16, 0x00);
				crc16 = ym_crc16_upd (crc16, 0x00);
				if (crc16 != ymodem_test_crc16_data [i].crc16)
						return FALSE;
		}
		return TRUE;
}

test_result_t ymodem_test (void)
{
		unsigned char ch, data [YM_BLK_SIZE_128BYTES];
		int i;

		test_assert (ymodem_test_crc16 () == TRUE);

		ym_tx_make_block_zero (&ymodem_instance, data, (unsigned int) sizeof (data), "TEST.TXT", (unsigned long) 123152);
		test_assert (util_memcmp (&data [0], "TEST.TXT", (unsigned int) 8) == 0 && data [8] == (unsigned char) '\0');
		test_assert (util_memcmp (&data [9], "123152", (unsigned int) 6) == 0 && data [15] == (unsigned char) '\0');

		ym_tx_make_block_null (&ymodem_instance, data, (unsigned int) sizeof (data));
		for (i = 0, ch = (unsigned char) 0; i < (int) sizeof (data); ++i)
				ch |= data [i];
		test_assert (ch == (unsigned char) 0);

		return TEST_RESULT_OKAY;
}

#endif

/* ----------------------------------------------------------------------------------------------------*/
