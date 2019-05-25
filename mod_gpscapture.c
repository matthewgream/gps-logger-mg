
/* ----------------------------------------------------------------------------------------------------*/
/*
	mod_gpscapture.c: gps logger module for gps position capturing.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#include "common.h"
#include "mod_util.h"
#include "mod_filefatfs.h"
#include "mod_gpscapture.h"

/* ----------------------------------------------------------------------------------------------------*/

typedef struct
{
		boolean suspended;
		fat_handle_t hndl;
		/*@notnull@*/ /*@observer@*/ const char * const name;
}
gps_capture_t;

static gps_capture_t gps_capture_instance = {
		FALSE,
		FAT_HANDLE_INVALID,
		(const char * const) "GPSPOSTN.TXT"
};

/* ----------------------------------------------------------------------------------------------------*/

static boolean gps_capture_file_open (gps_capture_t * const instance)
{
		if (!FAT_HANDLE_VALID (instance->hndl))
		{
				instance->hndl = fat_open (instance->name, FAT_OPT_WRITE|FAT_OPT_APPEND);

				if (!FAT_HANDLE_VALID (instance->hndl))
				{
						return FALSE;
				}
		}

		return TRUE;
}

static boolean gps_capture_file_close (gps_capture_t * const instance)
{
		if (FAT_HANDLE_VALID (instance->hndl))
		{
				(void) fat_close (instance->hndl);

				instance->hndl = FAT_HANDLE_INVALID;
		}

		return TRUE;
}

/*@observer@*/ /*@notnull@*/ static const char * gps_capture_position_get (void)
{
		return (const char * const) "TEMPORARY"; /*XXX*/
}

/* ----------------------------------------------------------------------------------------------------*/

boolean gps_capture_suspend (void)
{
		/*@shared@*/ gps_capture_t * const instance = &gps_capture_instance;

		instance->suspended = TRUE;

		(void) gps_capture_file_close (instance);

		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

boolean gps_capture_resume (void)
{
		/*@shared@*/ gps_capture_t * const instance = &gps_capture_instance;

		instance->suspended = FALSE;

		/* don't re-open, do it on demand */

		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

const char * gps_capture_filename (void)
{
		/*@shared@*/ gps_capture_t * const instance = &gps_capture_instance;

		return instance->name;
}

/* ----------------------------------------------------------------------------------------------------*/

boolean gps_capture_clear (void)
{
		/*@shared@*/ gps_capture_t * const instance = &gps_capture_instance;

		(void) gps_capture_file_close (instance);

		switch (fat_unlink (instance->name))
		{
				case FAT_ERR_FILENOTFOUND:
				case FAT_ERR_NONE:
						break;
				default:
						return FALSE;
		}

		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

const char * gps_capture_get (void)
{
		return gps_capture_position_get ();
}

/* ----------------------------------------------------------------------------------------------------*/

boolean gps_capture_save (const char * const description)
{
		/*@shared@*/ gps_capture_t * const instance = &gps_capture_instance;
		/*@shared@*/ const char * position;

		if (instance->suspended == TRUE)
				return FALSE;

		position = gps_capture_position_get ();

		if (!gps_capture_file_open (instance))
				return FALSE;

		if (fat_write (instance->hndl, (const unsigned char *) position, util_strlen (position), NULL) != FAT_ERR_NONE)
				return FALSE;
		if (description != NULL && description [0] != '\0')
		{
				if (fat_write (instance->hndl, (const unsigned char *) ",\"", (unsigned int) 2, NULL) != FAT_ERR_NONE)
						return FALSE;
				if (fat_write (instance->hndl, (const unsigned char *) description, util_strlen (description), NULL) != FAT_ERR_NONE)
						return FALSE;
				if (fat_write (instance->hndl, (const unsigned char *) "\"", (unsigned int) 1, NULL) != FAT_ERR_NONE)
						return FALSE;
		}
		if (fat_write (instance->hndl, (const unsigned char *) "\r\n", (unsigned int) 2, NULL) != FAT_ERR_NONE)
				return FALSE;
		if (fat_sync (instance->hndl) != FAT_ERR_NONE)
				return FALSE;

		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

#ifdef TEST_ENABLED

test_result_t gps_capture_test (void)
{
		/*@shared@*/ gps_capture_t * const instance = &gps_capture_instance;
		unsigned long filesize1, filesize2;
		fat_handle_t hndl;
		char buffer [96];
		unsigned int bufsiz;

		test_assert (gps_capture_filename () != NULL);

		test_assert (gps_capture_get () != NULL);

		test_assert (gps_capture_file_open (instance) == TRUE);
		test_assert (gps_capture_file_close (instance) == TRUE);
		test_assert (gps_capture_save ("test 1") == TRUE);
		test_assert (fat_tell (instance->hndl, &filesize1) == FAT_ERR_NONE && filesize1 > 0);
		test_assert (gps_capture_save ("test 2") == TRUE);
		test_assert (fat_tell (instance->hndl, &filesize2) == FAT_ERR_NONE && filesize2 > filesize1);
		test_assert (gps_capture_suspend () == TRUE);
		test_assert (gps_capture_save ("test 3") == FALSE);
		test_assert (gps_capture_resume () == TRUE);
		test_assert (gps_capture_save (NULL) == TRUE);
		test_assert (fat_tell (instance->hndl, &filesize1) == FAT_ERR_NONE && filesize1 > filesize2);
		test_assert (gps_capture_file_close (instance) == TRUE);
		DPRINTF (("-->\n"));
		test_assert (FAT_HANDLE_VALID (hndl = fat_open (gps_capture_filename (), FAT_OPT_READ)));
		do {
				(void) util_memset (buffer, (unsigned char) '\0', (unsigned int) sizeof (buffer));
				test_assert (fat_read (hndl, (unsigned char *) buffer, (unsigned int) (sizeof (buffer) - 1), &bufsiz) == FAT_ERR_NONE);
				DPRINTF (("%s", buffer));
		} while (bufsiz == (unsigned int) (sizeof (buffer) - 1));
		test_assert (fat_close (hndl) == FAT_ERR_NONE);
		DPRINTF (("<--\n"));
		test_assert (gps_capture_clear () == TRUE);
		test_assert (!FAT_HANDLE_VALID (hndl = fat_open (gps_capture_filename (), FAT_OPT_READ)));

		return TEST_RESULT_OKAY;
}

#endif

/* ----------------------------------------------------------------------------------------------------*/

