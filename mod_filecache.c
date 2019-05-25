
/* ----------------------------------------------------------------------------------------------------*/
/*
	mod_filecache.c: gps logger module for file cache processing.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#include "common.h"
#include "mod_util.h"
#include "mod_filefatfs.h"
#include "mod_filecache.h"

/* ----------------------------------------------------------------------------------------------------*/

typedef struct
{
		fat_handle_t handle;

		/*@shared@*/ unsigned char * buffer;
		unsigned int length;
		unsigned int offset_front;
		unsigned int offset_back;
		unsigned int offset_length;

		volatile boolean flush;
		/*@null@*/ void (*notify) (void);

		unsigned long timeout_normal;
		unsigned long timeout_quiet;
		util_timer_handle_t timeout_normal_handle;
		util_timer_handle_t timeout_quiet_handle;
		util_scheduler_handle_t timeout_scheduler_handle;
}
cachefile_t;

static cachefile_t cachefile_instance;

/* ----------------------------------------------------------------------------------------------------*/

static void cachefile_timeout_handler_timer (/*@notnull@*/ /*@shared@*/ void * const token)
{
		cachefile_t * const instance = (cachefile_t * const) token;

		assert (instance != NULL);

		instance->flush = TRUE; /* triggers the scheduler */
}

static void cachefile_timeout_normal_reset (/*@notnull@*/ /*@shared@*/ cachefile_t * const instance)
{
		assert (instance != NULL);

		if (instance->timeout_normal > (unsigned long) 0 && instance->length > (unsigned int) 0 && UTIL_TIMER_HANDLE_VALID (instance->timeout_normal_handle))
		{
				util_timer_restart (instance->timeout_normal_handle);
		}
}

static void cachefile_timeout_quiet_reset (/*@notnull@*/ /*@shared@*/ cachefile_t * const instance)
{
		assert (instance != NULL);

		if (instance->timeout_quiet > (unsigned long) 0 && instance->length > (unsigned int) 0 && UTIL_TIMER_HANDLE_VALID (instance->timeout_quiet_handle))
		{
				util_timer_restart (instance->timeout_quiet_handle);
		}
}

static void cachefile_timeout_normal_start (/*@notnull@*/ /*@shared@*/ cachefile_t * const instance)
{
		assert (instance != NULL);

		if (instance->timeout_normal > (unsigned long) 0 && instance->length > (unsigned int) 0)
		{
				instance->timeout_normal_handle = util_timer_create (cachefile_timeout_handler_timer, (void *) instance, instance->timeout_normal);
		}
}

static void cachefile_timeout_quiet_start (/*@notnull@*/ /*@shared@*/ cachefile_t * const instance)
{
		assert (instance != NULL);

		if (instance->timeout_quiet > (unsigned long) 0 && instance->length > (unsigned int) 0)
		{
				instance->timeout_quiet_handle = util_timer_create (cachefile_timeout_handler_timer, (void *) instance, instance->timeout_quiet);
		}
}

static void cachefile_timeout_normal_stop (/*@notnull@*/ /*@shared@*/ cachefile_t * const instance)
{
		assert (instance != NULL);

		if (instance->timeout_normal > (unsigned long) 0 && instance->length > (unsigned int) 0 && UTIL_TIMER_HANDLE_VALID (instance->timeout_normal_handle))
		{
				util_timer_destroy (instance->timeout_normal_handle);
				instance->timeout_normal_handle = UTIL_TIMER_HANDLE_INVALID;
		}
}

static void cachefile_timeout_quiet_stop (/*@notnull@*/ /*@shared@*/ cachefile_t * const instance)
{
		assert (instance != NULL);

		if (instance->timeout_quiet > (unsigned long) 0 && instance->length > (unsigned int) 0 && UTIL_TIMER_HANDLE_VALID (instance->timeout_quiet_handle))
		{
				util_timer_destroy (instance->timeout_quiet_handle);
				instance->timeout_quiet_handle = UTIL_TIMER_HANDLE_INVALID;
		}
}

/* ----------------------------------------------------------------------------------------------------*/

static boolean cachefile_flush (/*@notnull@*/ /*@shared@*/ cachefile_t * const instance, const unsigned int write_size_reqd, const boolean timer_reset);

static boolean cachefile_timeout_handler_scheduler (/*@notnull@*/ /*@shared@*/ void * const token)
{
		cachefile_t * const instance = (cachefile_t * const) token;

		assert (instance != NULL);

		if (cachefile_flush (instance, (unsigned int) 0, FALSE) == FALSE)
		{
				DPRINTF (("cachefile_timer_handler_scheduler: cachefile_flush failed\n"));
				return FALSE;
		}

		cachefile_timeout_normal_reset (instance);
		cachefile_timeout_quiet_reset (instance);

		return TRUE;
}

static void cachefile_timeout_scheduler_start (/*@notnull@*/ /*@shared@*/ cachefile_t * const instance)
{
		assert (instance != NULL);

		if ((instance->timeout_quiet > (unsigned long) 0 || instance->timeout_normal > (unsigned long) 0) && instance->length > (unsigned int) 0)
		{
				instance->timeout_scheduler_handle = util_scheduler_create (cachefile_timeout_handler_scheduler, (void *) instance, &instance->flush);
		}
}

static void cachefile_timeout_scheduler_stop (/*@notnull@*/ /*@shared@*/ cachefile_t * const instance)
{
		assert (instance != NULL);

		if ((instance->timeout_quiet > (unsigned long) 0 || instance->timeout_normal > (unsigned long) 0) &&
					instance->length > (unsigned int) 0 && UTIL_SCHEDULER_HANDLE_VALID (instance->timeout_scheduler_handle))
		{
				util_scheduler_destroy (instance->timeout_scheduler_handle);
				instance->timeout_scheduler_handle = UTIL_SCHEDULER_HANDLE_INVALID;
		}
}

/* ----------------------------------------------------------------------------------------------------*/

static boolean cachefile_flush (/*@notnull@*/ /*@shared@*/ cachefile_t * const instance, const unsigned int write_size_reqd, const boolean timer_reset)
{
		assert (instance != NULL);

		if (instance->offset_length > (unsigned int) 0 && instance->offset_length >= write_size_reqd)
		{
				unsigned int length = (write_size_reqd == (unsigned int) 0) ? instance->offset_length : write_size_reqd;

				assert (length > (unsigned int) 0);

				(void) fat_resume ();

				if ((instance->offset_back + length) <= instance->length)
				{
						unsigned int actual;

						if (fat_write (instance->handle, &instance->buffer [instance->offset_back], length, &actual) != FAT_ERR_NONE || actual != length)
						{
								(void) fat_suspend ();

								return FALSE;
						}
				}
				else
				{
						unsigned int offset = instance->length - instance->offset_back;
						unsigned int actual;

						assert (offset > (unsigned int) 0 && (length - offset) > (unsigned int) 0);

						if ((fat_write (instance->handle, &instance->buffer [instance->offset_back], offset, &actual) != FAT_ERR_NONE || actual != offset) ||
							(fat_write (instance->handle, &instance->buffer [0], (length - offset), &actual) != FAT_ERR_NONE || actual != (length - offset)))
						{
								(void) fat_suspend ();

								return FALSE;
						}
				}

				if (fat_flush () != FAT_ERR_NONE)
				{
						(void) fat_suspend ();

						return FALSE;
				}

				(void) fat_suspend ();

				instance->offset_back += length;
				while (instance->offset_back >= instance->length)
						instance->offset_back -= instance->length;
				instance->offset_length -= length;

				if (timer_reset)
				{
						cachefile_timeout_normal_reset (instance);
				}

				if (instance->notify != NULL)
				{
						(*instance->notify) ();
				}
		}

		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

boolean cachefile_init (const cachefile_config_t * const config)
{
		/*@shared@*/ cachefile_t * const instance = &cachefile_instance;
		unsigned char * buffer;

		assert (config != NULL);

		if ((buffer = util_buffer_alloc (config->buffer_size)) == NULL)
		{
				return FALSE;
		}

		instance->buffer = buffer;
		instance->length = config->buffer_size;
		instance->timeout_normal = config->timeout_normal;
		instance->timeout_quiet = config->timeout_quiet;
		instance->notify = config->notify_write;

		DPRINTF (("cachefile_init: length=%u, timeout_n=%lu, timeout_q=%lu\n", 
					instance->length, instance->timeout_normal, instance->timeout_quiet));

		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

boolean cachefile_term (void)
{
		/*cachefile_t * const instance = &cachefile_instance;*/

		util_buffer_reset ();

		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

boolean cachefile_attach (fat_handle_t handle)
{
		/*@shared@*/ cachefile_t * const instance = &cachefile_instance;

		assert (FAT_HANDLE_VALID (handle));

		instance->handle = handle;
		instance->offset_front = (unsigned int) 0;
		instance->offset_back = (unsigned int) 0;
		instance->offset_length = (unsigned int) 0;

		instance->flush = FALSE;
		cachefile_timeout_quiet_start (instance);
		cachefile_timeout_normal_start (instance);
		cachefile_timeout_scheduler_start (instance);

		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

boolean cachefile_detach (void)
{
		/*@shared@*/ cachefile_t * const instance = &cachefile_instance;
		boolean status;

		status = cachefile_flush (instance, (unsigned int) 0, FALSE);

		cachefile_timeout_scheduler_stop (instance);
		cachefile_timeout_quiet_stop (instance);
		cachefile_timeout_normal_stop (instance);

		instance->handle = FAT_HANDLE_INVALID;

		return status;
}

/* ----------------------------------------------------------------------------------------------------*/

boolean cachefile_write (const unsigned char * const buffer, const unsigned int length)
{
		/*@shared@*/ cachefile_t * const instance = &cachefile_instance;

		assert (buffer != NULL);
		assert (length > 0);

		if (instance->length == (unsigned int) 0)
		{
				unsigned int actual;

				(void) fat_resume ();

				if (fat_write (instance->handle, buffer, length, &actual) != FAT_ERR_NONE || actual != length)
				{
						(void) fat_suspend ();

						return FALSE;
				}

				if (fat_flush () != FAT_ERR_NONE)
				{
						(void) fat_suspend ();

						return FALSE;
				}

				(void) fat_suspend ();

				if (instance->notify != NULL)
				{
						(*instance->notify) ();
				}
		}
		else
		{		unsigned int offset = (unsigned int) 0;

				while (offset < length)
				{
						unsigned int length_insert;

						/* if there's no available space: could occur if previous flush failed! */
						if (instance->offset_length == instance->length && cachefile_flush (instance, (unsigned int) 0, TRUE) == FALSE)
						{
								return FALSE;
						}

						length_insert = (length - offset) > (instance->length - instance->offset_length) ? 
								(instance->length - instance->offset_length) : (length - offset);
						assert (length_insert > (unsigned int) 0);

						if ((instance->offset_front + length_insert) <= instance->length)
						{
								(void) util_memcpy (&instance->buffer [instance->offset_front], &buffer [offset], length_insert);
						}
						else
						{
								unsigned int length_partial = (instance->length - instance->offset_front);

								assert (length_partial > (unsigned int) 0);

								(void) util_memcpy (&instance->buffer [instance->offset_front], &buffer [offset], length_partial);
								(void) util_memcpy (&instance->buffer [0], &buffer [offset + (instance->length - instance->offset_front)],
										length_insert - length_partial);
						}

						instance->offset_front += length_insert;
						while (instance->offset_front >= instance->length)
								instance->offset_front -=  instance->length;
						instance->offset_length += length_insert;

						if (cachefile_flush (instance, instance->length, TRUE) == FALSE)
						{
								DPRINTF (("cachefile_write: cachefile_flush failed\n"));
								return FALSE;
						}

						offset += length_insert;
				}

				cachefile_timeout_quiet_reset (instance);
		}

		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

#ifdef TEST_ENABLED

test_result_t cachefile_test (void)
{
		unsigned int fileindex;
		char filename [13];
		fat_handle_t handle;
		cachefile_config_t config;
		unsigned int offset, length = (unsigned int) 8192;
		unsigned char buffer [128];
		unsigned long filesize;

		for (offset = 0; offset < (unsigned int) sizeof (buffer); offset++)
				buffer [offset] = (unsigned char) (offset & 0xFF);

		/* cachefile_init */
		config.buffer_size = length;
		config.timeout_normal = (unsigned long) 5;
		config.timeout_quiet = (unsigned long) 5;
		config.notify_write = NULL;
		test_assert (cachefile_init (&config) == TRUE);

		/* cachefile_attach */
		for (fileindex = 0; fileindex < (unsigned int) 990; fileindex++) {
				*filename = '\0';
				(void) util_makenumberedfn (filename, "TEST", "TMP", fileindex);
				handle = fat_open (filename, FAT_OPT_READ);
				if (!FAT_HANDLE_VALID (handle))
						break;
				test_assert (fat_close (handle) == FAT_ERR_NONE);
		}
		handle = fat_open (filename, FAT_OPT_WRITE);
		test_assert (FAT_HANDLE_VALID (handle));
		(void) fat_suspend ();
		test_assert (cachefile_attach (handle) == TRUE);

		/* cachefile_write */
		offset = 0;
		while (offset < (length - (unsigned int) sizeof (buffer))) {
				unsigned int current = (unsigned int) ((util_rand () % (sizeof (buffer) - (unsigned int) 1)) + (unsigned int) 1);
				*buffer = '\0';
				test_assert (cachefile_write (buffer, current) == TRUE);
				offset += current;
		}
		test_assert (fat_tell (handle, &filesize) == FAT_ERR_NONE && filesize == (unsigned long) 0);
		while (offset < (length + (unsigned int) 16)) {
				*buffer = '\0';
				test_assert (cachefile_write (buffer, (unsigned int) 1) == TRUE);
				offset += 1;
		}

		/* cachefile_timeouts */
		test_assert (fat_tell (handle, &filesize) == FAT_ERR_NONE && filesize == (unsigned long) length);
		/*@i@*/ while (fat_tell (handle, &filesize) == FAT_ERR_NONE && filesize == (unsigned long) length)
				(void) util_scheduler_process ();
		test_assert (fat_tell (handle, &filesize) == FAT_ERR_NONE && filesize == (unsigned long) offset);

		/* cachefile_detach */
		*buffer = '\0';
		test_assert (cachefile_write (buffer, (unsigned int) sizeof (buffer)) == TRUE);
		test_assert (fat_tell (handle, &filesize) == FAT_ERR_NONE && filesize == (unsigned long) offset);
		test_assert (cachefile_detach () == TRUE);
		test_assert (fat_tell (handle, &filesize) == FAT_ERR_NONE && filesize == (unsigned long) ((unsigned int) offset + (unsigned int) sizeof (buffer)));
		(void) fat_resume ();
		test_assert (fat_close (handle) == FAT_ERR_NONE);
		test_assert (fat_unlink (filename) == FAT_ERR_NONE);

		/* cachefile_term */
		test_assert (cachefile_term () == TRUE);

		return TEST_RESULT_OKAY;
}

#endif

/* ----------------------------------------------------------------------------------------------------*/

