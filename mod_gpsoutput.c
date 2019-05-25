
/* ----------------------------------------------------------------------------------------------------*/
/*
	mod_gpsoutput.c: gps logger module for GPS processing.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#include "common.h"
#include "mod_util.h"
#include "mod_lpc21XXhal.h"
#include "mod_filefatfs.h"
#include "mod_filecache.h"
#include "mod_packlzari.h"
#include "mod_gpsoutput.h"

/* ----------------------------------------------------------------------------------------------------*/

static fat_handle_t gps_file_open (void)
{
		unsigned int indax = 0;
		char name [13];
		fat_handle_t handle;

		(void) fat_resume ();

		do
		{
				*name = '\0';
				(void) util_makenumberedfn (name, "GPS", "TXT", indax);
				handle = fat_open (name, FAT_OPT_WRITE);
		}
		while (!FAT_HANDLE_VALID (handle) && indax++ < (unsigned int) 100000);

		(void) fat_suspend ();

		if (FAT_HANDLE_VALID (handle))
		{
				if (cachefile_attach (handle) == FALSE)
				{
						(void) fat_resume ();

						(void) fat_close (handle);

						(void) fat_suspend ();

						return FAT_HANDLE_INVALID;
				}

				DPRINTF (("gps_file = %s\n", name));
		}

		return handle;
}

static void gps_file_close (fat_handle_t handle)
{
		if (FAT_HANDLE_VALID (handle))
		{
				(void) fat_suspend ();

				(void) cachefile_detach ();

				(void) fat_resume ();

				(void) fat_close (handle);

				(void) fat_flush ();
		}
}

static void gps_file_update (void)
{
		/* leds: v1.0 toggle stat1, v2.4: toggle green */
		lpc21XX_led_toggle ( (unsigned char) 1);
}

static boolean gps_file_write (fat_handle_t handle, const unsigned char * const buffer, const unsigned int length)
{
		assert (FAT_HANDLE_VALID (handle));
		assert (buffer != NULL);
		assert (length > 0);

		UNUSED (handle);

		return cachefile_write (buffer, length);
}

static boolean gps_file_init (const gps_config_mode_file_t * const config)
{
		cachefile_config_t cachefile_config;

		assert (config != NULL);

		cachefile_config.buffer_size = config->buffer_size;
		cachefile_config.timeout_normal = config->timeout_normal;
		cachefile_config.timeout_quiet = config->timeout_quiet;
		cachefile_config.notify_write = gps_file_update;

		if (cachefile_init (&cachefile_config) == FALSE)
		{
				return FALSE;
		}

		return TRUE;
}

static boolean gps_file_term (void)
{
		(void) cachefile_term (); /*XXX*/

		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

static void gps_pass_write (const unsigned char * const buffer, const unsigned int length)
{
		assert (buffer != NULL);
		assert (length > 0);

		lpc21XX_uart0_write (buffer, length);
}

static boolean gps_pass_init (const gps_config_mode_pass_t * const config)
{
#ifndef DEBUG
		unsigned int rate = config->serial_speed;
#else
		UNUSED (config);
#endif

		assert (config != NULL);

#ifndef DEBUG
		lpc21XX_uart0_init (rate, FALSE);
#endif

		return TRUE;
}

static boolean gps_pass_term (void)
{
#ifndef DEBUG
		lpc21XX_uart0_term ();
#endif

		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

static gps_config_output_t gps_output_config;
static fat_handle_t gps_output_file_handle;

static boolean gps_output_write_file (const unsigned char* const buffer, const unsigned int length)
{
		if (!FAT_HANDLE_VALID (gps_output_file_handle))
		{
				gps_output_file_handle = gps_file_open ();

				if (!FAT_HANDLE_VALID (gps_output_file_handle))
				{
						return FALSE;
				}
		}

		if (gps_file_write (gps_output_file_handle, buffer, length) == FALSE)
		{
				return FALSE;
		}

		return TRUE;
}

boolean gps_output_write (const unsigned char* const buffer, const unsigned int length)
{
		if (gps_output_config.file.enabled)
		{
				if (gps_output_config.compress)
				{
						if (packlzari_encode_write (buffer, length) == FALSE)
						{
								return FALSE;
						}
				}
				else
				{
						if (gps_output_write_file (buffer, length) == FALSE)
						{
								return FALSE;
						}
				}
		}

		if (gps_output_config.pass.enabled)
		{
				gps_pass_write (buffer, length);
		}

		return TRUE;
}

boolean gps_output_init (const gps_config_output_t * const config)
{
		assert (config != NULL);

		DPRINTF (("gps_output_init\n"));

		(void) util_memcpy ((void *) &gps_output_config, (const void *) config, (unsigned int) sizeof (gps_output_config));

		if (gps_output_config.file.enabled)
		{
				DPRINTF (("gps_mode = file\n"));
		
				if (gps_file_init (&gps_output_config.file) == FALSE)
				{
						return FALSE;
				}

				gps_output_file_handle = FAT_HANDLE_INVALID;
		}

		if (gps_output_config.pass.enabled)
		{
				DPRINTF (("gps_mode = pass\n"));

				if (gps_pass_init (&gps_output_config.pass) == FALSE)
				{
						return FALSE;
				}
		}

		if (gps_output_config.compress)
		{
				if (packlzari_encode_init (gps_output_write_file) == FALSE)
				{
						return FALSE;
				}
		}

		return TRUE;
}

boolean gps_output_term (void)
{
		DPRINTF (("gps_output_term\n"));

		if (gps_output_config.compress)
		{
				(void) packlzari_encode_term ();
		}

		if (gps_output_config.file.enabled)
		{
				(void) gps_file_close (gps_output_file_handle);

				(void) gps_file_term ();
		}

		if (gps_output_config.pass.enabled)
		{
				(void) gps_pass_term ();
		}

		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

#ifdef TEST_ENABLED

test_result_t gps_output_test (void) /* XXX */
{
		return TEST_RESULT_OKAY;
}

#endif

/* ----------------------------------------------------------------------------------------------------*/

