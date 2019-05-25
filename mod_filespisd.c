
/* ----------------------------------------------------------------------------------------------------*/
/*
	mod_filespisd.c: gps logger module for SPI based SD card access.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#include "common.h"
#include "mod_lpc21XXhal.h"
#include "mod_util.h"
#include "mod_filespisd.h"

/* ----------------------------------------------------------------------------------------------------*/

#define CMD0  ((unsigned char) 0x40)	  /* reset */
#define CMD1  ((unsigned char) 0x41)	  /* idle -> active */
#define CMD9  ((unsigned char) 0x49)	  /* read CSD (card specific data register) */
#define CMD10 ((unsigned char) 0x4A)	  /* read CID (card identification register) */
#define CMD12 ((unsigned char) 0x4C)	  /* read multiple-block terminate */
#define CMD13 ((unsigned char) 0x4D)	  /* read SR (status register) */
#define CMD16 ((unsigned char) 0x50)	  /* set block length */
#define CMD17 ((unsigned char) 0x51)	  /* read single-block */
#define CMD18 ((unsigned char) 0x52)	  /* read multiple-block */
#define CMD24 ((unsigned char) 0x58)	  /* write single-block */
#define CMD25 ((unsigned char) 0x59)	  /* write multiple-block */
#define CMD27 ((unsigned char) 0x5B)	  /* write CSD (card specific data) */
#define CMD30 ((unsigned char) 0x5E)	  /* read write-protection bits */
#define CMD58 ((unsigned char) 0x7A)	  /* read OCR (operation conditions register) */
#define CMD59 ((unsigned char) 0x7B)	  /* set CRC mode */

/* ----------------------------------------------------------------------------------------------------*/

static boolean card_valid = FALSE;

/* ----------------------------------------------------------------------------------------------------*/

static inline void card_cmd_request (const unsigned char cmd, const unsigned char arg_a, const unsigned char arg_b, const unsigned char arg_c, const unsigned char arg_d, const unsigned char crc)
{
		lpc21XX_spi_put (cmd);
		lpc21XX_spi_put (arg_a);
		lpc21XX_spi_put (arg_b);
		lpc21XX_spi_put (arg_c);
		lpc21XX_spi_put (arg_d);
		lpc21XX_spi_put (crc);
}

static inline boolean card_cmd_response (const unsigned char res)
{
		unsigned int count;

		count = (unsigned int) 256;
		while (count-- > 0)
		{
				if (lpc21XX_spi_get () == res)
				{
						return TRUE;
				}
		}

		return FALSE;
}

static inline boolean card_state_check_write (void)
{
		unsigned int count;

		count = (unsigned int) 64;
		while (count-- > 0)
		{
				unsigned char ch = (lpc21XX_spi_get () & 0x1F);
				if (ch == (unsigned char) 0x05)
						break;
				if (ch == (unsigned char) 0x0B || ch == (unsigned char) 0x0D)
				{
						return FALSE;
				}
		}

		count = (unsigned int) 0xFFFF;
		while (count-- > 0)
		{
				if (lpc21XX_spi_get () != (unsigned char) 0x00)
				{
						return TRUE;
				}
		}

		return FALSE;
}

static inline boolean card_state_check_read (void)
{
		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

boolean card_init (void)
{
		DPRINTF (("card_init\n"));

		lpc21XX_spi_init ();

		card_valid = TRUE;

		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

boolean card_term (void)
{
		DPRINTF (("card_term\n"));

		lpc21XX_spi_term ();

		card_valid = FALSE;

		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

boolean card_detect (void)
{
		return card_valid;
}

/* ----------------------------------------------------------------------------------------------------*/

boolean card_enable (void)
{
		boolean status = FALSE;
		unsigned int count;

		lpc21XX_spi_release ();
		count = (unsigned int) 10;
		while (count-- > 0)
		{
				lpc21XX_spi_put ((unsigned char) 0xFF);
		}

		lpc21XX_spi_acquire ();
		card_cmd_request (CMD0, (unsigned char) 0x00, (unsigned char) 0x00,
								(unsigned char) 0x00, (unsigned char) 0x00,
								(unsigned char) 0x95);
		if (card_cmd_response ((unsigned char) 0x01))
		{
				count = (unsigned int) 256;
				while (count-- > 0 && !status)
				{
						card_cmd_request (CMD1, (unsigned char) 0x00, (unsigned char) 0x00,
												(unsigned char) 0x00, (unsigned char) 0x00,
												(unsigned char) 0xFF);
						if (card_cmd_response ((unsigned char) 0x00))
						{
								status = TRUE;
						}
				}
		}
		lpc21XX_spi_release ();
		(void) lpc21XX_spi_get (); /*XXX really? after release */

		card_valid = status;

		return card_valid;
}

/* ----------------------------------------------------------------------------------------------------*/

boolean card_config_blocklength (void)
{
		boolean status = FALSE;

		lpc21XX_spi_acquire ();
		card_cmd_request (CMD16, (unsigned char) ((CARD_SECTOR_SIZE>>24) & 0xFF), (unsigned char) ((CARD_SECTOR_SIZE>>16) & 0xFF),
								 (unsigned char) ((CARD_SECTOR_SIZE>>8) & 0xFF), (unsigned char) (CARD_SECTOR_SIZE & 0xFF),
								 (unsigned char) 0xFF);
		if (card_cmd_response ((unsigned char) 0x00))
		{
				status = TRUE;
		}
		lpc21XX_spi_release ();
		(void) lpc21XX_spi_get (); /*XXX really? after release */

		return status;
}

/* ----------------------------------------------------------------------------------------------------*/

boolean card_state_suspend (void)
{
		lpc21XX_spi_term ();

		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

boolean card_state_resume (void)
{
		lpc21XX_spi_init ();

		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

unsigned int card_sector_read (const unsigned long sector, unsigned char * const buffer)
{
		assert (buffer != NULL);

		lpc21XX_spi_acquire ();
		card_cmd_request (CMD17, (unsigned char) (((sector<<1)>>16) & 0xFF), (unsigned char) (((sector<<1)>>8) & 0xFF),
								 (unsigned char) ((sector<<1) & 0xFF), (unsigned char) 0x00,
								 (unsigned char) 0xFF);
		if (card_cmd_response ((unsigned char) 0x00))
		{
				if (card_cmd_response ((unsigned char) 0xFE))
				{
						unsigned int count = (unsigned int) CARD_SECTOR_SIZE;
						unsigned char * pointr = buffer;

						while (count-- > 0)
								*pointr++ = lpc21XX_spi_get ();

						(void) lpc21XX_spi_get ();
						(void) lpc21XX_spi_get ();
						card_valid = card_state_check_read ();
				}
		}
		lpc21XX_spi_release ();
		(void) lpc21XX_spi_get (); /*XXX really? after release */

		return (unsigned int) (card_valid ? CARD_SECTOR_SIZE : 0);
}

/* ----------------------------------------------------------------------------------------------------*/

unsigned int card_sector_write (const unsigned long sector, const unsigned char * const buffer)
{
		assert (buffer != NULL);

		lpc21XX_spi_acquire ();
		card_cmd_request (CMD24, (unsigned char) (((sector<<1)>>16) & 0xFF), (unsigned char) (((sector<<1)>>8) & 0xFF),
								 (unsigned char) ((sector<<1) & 0xFF), (unsigned char) 0x00,
								 (unsigned char) 0xFF);
		if (card_cmd_response ((unsigned char) 0x00))
		{
				lpc21XX_spi_put ((unsigned char) 0xFE);
				{
						unsigned int count = (unsigned int) CARD_SECTOR_SIZE;
						const unsigned char * pointr = buffer;

						while (count-- > 0)
								lpc21XX_spi_put (*pointr++);

						lpc21XX_spi_put ((unsigned char) 0xFF);
						lpc21XX_spi_put ((unsigned char) 0xFF);
						card_valid = card_state_check_write ();
				}
		}
		lpc21XX_spi_release ();
		(void) lpc21XX_spi_get (); /*XXX really? after release */

		return (unsigned int) (card_valid ? CARD_SECTOR_SIZE : 0);
}

/* ----------------------------------------------------------------------------------------------------*/

#ifdef TEST_ENABLED

test_result_t card_test (void)
{
		unsigned char* bptrA = util_buffer_alloc ((unsigned int) CARD_SECTOR_SIZE);
		unsigned char* bptrB = util_buffer_alloc ((unsigned int) CARD_SECTOR_SIZE);
		unsigned int i;

		test_assert (bptrA != NULL && bptrB != NULL);

		/* card_init already called */
#ifdef EMULATE
		{	/*@i@*/ extern char * emu_file_dir;
			if (emu_file_dir != NULL) test_assert (card_init ());
		}
#endif

		/* card control */
		test_assert (card_enable ());
		test_assert (card_detect ());
		test_assert (card_config_blocklength ());
		test_assert (card_state_suspend ());
		test_assert (card_state_resume ());

		/* card xfer read */
		test_assert (card_sector_read ((unsigned long) 1, bptrA) == (unsigned int) CARD_SECTOR_SIZE);
		test_assert (card_sector_read ((unsigned long) 1, bptrB) == (unsigned int) CARD_SECTOR_SIZE);
		test_assert (util_memcmp (&bptrA [0], &bptrB [0], (unsigned int) CARD_SECTOR_SIZE) == (unsigned int) 0);

		/* card xfer write */
		test_assert (util_memcmp (&bptrA [0], &bptrB [0], (unsigned int) CARD_SECTOR_SIZE) == (unsigned int) 0);
		for (i = (unsigned int) 0; i < (unsigned int) CARD_SECTOR_SIZE; i++)
				bptrB [i] ^= 0xFF;
		test_assert (util_memcmp (&bptrA [0], &bptrB [0], (unsigned int) CARD_SECTOR_SIZE) != (unsigned int) 0);
		test_assert (card_sector_write ((unsigned long) 1, bptrB) == (unsigned int) CARD_SECTOR_SIZE);
		test_assert (card_state_suspend ());
		test_assert (card_state_resume ());
		test_assert (card_sector_read ((unsigned long) 1, bptrA) == (unsigned int) CARD_SECTOR_SIZE);
		test_assert (util_memcmp (&bptrA [0], &bptrB [0], (unsigned int) CARD_SECTOR_SIZE) == 0);
		for (i = (unsigned int) 0; i < (unsigned int) CARD_SECTOR_SIZE; i++)
				bptrA [i] ^= 0xFF;
		test_assert (card_sector_write ((unsigned long) 1, bptrA) == (unsigned int) CARD_SECTOR_SIZE);

		util_buffer_reset ();

		return TEST_RESULT_OKAY;
}

#endif

/* ----------------------------------------------------------------------------------------------------*/
