
/* ----------------------------------------------------------------------------------------------------*/
/*
	mod_gps.c: gps logger module for GPS processing.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#include "common.h"
#include "mod_util.h"
#include "mod_lpc21XXhal.h"
#include "mod_gps.h"

/* ----------------------------------------------------------------------------------------------------*/

#define GPS_NMEA_BUFFER_SIZE  (79+1+2+15)

/* ----------------------------------------------------------------------------------------------------*/

/* XXX cleanup state variables */
static util_scheduler_handle_t gps_scheduler_handle;

static boolean gps_exec (void * const token)
{
		unsigned char buffer [GPS_NMEA_BUFFER_SIZE];
		unsigned int length;

		UNUSED (token);

		if (gps_input_read (buffer, (unsigned int) sizeof (buffer)) > 0)
		{
				DPRINTF (("--GPS--> %s <----\n", buffer));

				if ((length = gps_format_process (buffer, (unsigned int) sizeof (buffer))) > 0)
				{
						if (gps_output_write (buffer, length) == FALSE)
						{
								DPRINTF (("gps_exec: gps_output_write failed\n"));
								return FALSE;
						}
				}
		}

		return TRUE;
}

boolean gps_init (const gps_config_t * const config)
{
		assert (config != NULL);

		DPRINTF (("gps_init\n"));

		if (gps_output_init (&config->output) == FALSE)
		{
				return FALSE;
		}

		if (gps_format_init (&config->format) == FALSE)
		{
				return FALSE;
		}

		if (gps_process_init (&config->process) == FALSE)
		{
				return FALSE;
		}

		if (gps_input_init (&config->input) == FALSE)
		{
				return FALSE;
		}

		gps_scheduler_handle = util_scheduler_create (gps_exec, NULL, NULL);
		if (!UTIL_SCHEDULER_HANDLE_VALID (gps_scheduler_handle))
		{
				return FALSE;
		}

		return TRUE;
}

boolean gps_term (void)
{
		unsigned char buffer [GPS_NMEA_BUFFER_SIZE];
		unsigned int length;

		DPRINTF (("gps_term\n"));

		util_scheduler_destroy (gps_scheduler_handle);

		(void) gps_process_term ();

		if ((length = gps_format_term (buffer, (unsigned int) sizeof (buffer))) > 0)
		{
				if (gps_output_write (buffer, length) == FALSE)
				{
						return FALSE;
				}
		}

		if (gps_output_term () == FALSE)
		{
				return FALSE;
		}

		(void) gps_input_term ();

		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

#ifdef TEST_ENABLED

test_result_t gps_test (void) /* XXX */
{
		return TEST_RESULT_OKAY;
}

#endif

/* ----------------------------------------------------------------------------------------------------*/

