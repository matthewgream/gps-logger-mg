
/* ----------------------------------------------------------------------------------------------------*/
/*
	mod_lpc21XXemu.c: gps logger module for LPC21XX hardware emulation.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#include "common.h"
#include "mod_lpc21XXhal.h"

#ifdef linux
/*@-namechecks@*/
#define _XOPEN_SOURCE 500
/*@=namechecks@*/
#endif

/*@-incondefs@*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef linux
#ifdef SPLINT
#include <bits/sigset.h>
#endif
#endif
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/file.h>
/*@=incondefs@*/

#ifdef linux
/*@-declundef@*/
/*@-exportheader@*/
extern int pthread_yield (void);
/*@=exportheader@*/
/*@=declundef@*/
#endif

/* ----------------------------------------------------------------------------------------------------*/
/* emu_timer */

struct st_emu_timer_t;
typedef void (*emu_callback_t) (/*@notnull@*/ struct st_emu_timer_t*, long);
typedef struct st_emu_timer_t {
		/*@notnull@*/ /*@shared@*/ emu_callback_t callback;
		long secs;
		boolean repeat;
		long x_rsecs;
		/*@null@*/ /*@shared@*/ struct st_emu_timer_t* x_next;
} emu_timer_t;
/*@shared@*/ /*@null@*/ static emu_timer_t* emu_timer_queue = NULL;
static unsigned long emu_timer_scale = 0;
static pthread_mutex_t emu_timer_lock;
static pthread_t emu_timer_thread;
static volatile boolean emu_timer_isrunning = FALSE;
static volatile boolean emu_timer_isticking = FALSE;
static volatile boolean emu_timer_suspended = FALSE;
static void emu_timer_enqueue (/*@shared@*/ emu_timer_t* ctx, boolean locked)
{
		assert (ctx != NULL);

		if (!locked)
				(void) pthread_mutex_lock (&emu_timer_lock);
		do {
				/*@shared@*/ emu_timer_t* qp = NULL;
				/*@shared@*/ emu_timer_t* qc = emu_timer_queue;
				ctx->x_rsecs = ctx->secs;
				while (qc != NULL && qc->x_rsecs < ctx->x_rsecs) {
						ctx->x_rsecs -= qc->x_rsecs;
						qp = qc;
						qc = qc->x_next;
				}
				if (qp == NULL)
						emu_timer_queue = ctx;
				else
						qp->x_next = ctx;
				ctx->x_next = qc;
				if (ctx->x_next != NULL)
						ctx->x_next->x_rsecs -= ctx->x_rsecs;
		} while (FALSE);
		if (!locked)
				(void) pthread_mutex_unlock (&emu_timer_lock);
		assert (emu_timer_queue != NULL);
}
static void emu_timer_dequeue (/*@shared@*/ emu_timer_t* ctx)
{
		assert (ctx != NULL);

		(void) pthread_mutex_lock (&emu_timer_lock);
		do {
				/*@shared@*/ emu_timer_t* qp = NULL;
				/*@shared@*/ emu_timer_t* qc = emu_timer_queue;
				while (qc != NULL && qc != ctx) {
						qp = qc;
						qc = qc->x_next;
				}
				if (qc == NULL)
						break;
				if (qp == NULL)
						emu_timer_queue = ctx->x_next;
				else
						qp->x_next = ctx->x_next;
				if (ctx->x_next != NULL)
						ctx->x_next->x_rsecs += ctx->x_rsecs;
		} while (FALSE);
		(void) pthread_mutex_unlock (&emu_timer_lock);
}
static void emu_timer_tick (void)
{
		(void) pthread_mutex_lock (&emu_timer_lock);
		do {
				if (emu_timer_queue != NULL) {
						--(emu_timer_queue->x_rsecs);
						while (emu_timer_queue != NULL && emu_timer_queue->x_rsecs == 0) {
								/*@shared@*/ emu_timer_t* ctx = emu_timer_queue;
								emu_timer_queue = emu_timer_queue->x_next;
								assert (ctx != NULL);
								if (ctx->repeat)
										emu_timer_enqueue (ctx, TRUE);
								(void) pthread_mutex_unlock (&emu_timer_lock);
								if (ctx->callback != NULL)
										(*ctx->callback)(ctx, ctx->secs);
								(void) pthread_mutex_lock (&emu_timer_lock);
						}
				}
		} while (FALSE);
		(void) pthread_mutex_unlock (&emu_timer_lock);
}
/*@null@*/ /*@shared@*/ static emu_timer_t* emu_timer_create (emu_callback_t callback, long secs, boolean repeat)
{
		/*@only@*/ emu_timer_t * ctx = (emu_timer_t *) malloc (sizeof (*ctx));
		assert (ctx != NULL);
		assert (callback != NULL);
		ctx->callback = callback;
		ctx->secs = secs;
		ctx->repeat = repeat;
		ctx->x_rsecs = 0;
		ctx->x_next = NULL;
		emu_timer_enqueue (ctx, FALSE);
		return ctx;
}
static void emu_timer_destroy (/*@notnull@*/ /*@shared@*/ emu_timer_t* ctx)
{
		assert (ctx != NULL);
		emu_timer_dequeue (ctx);
		/*@i@*/ free ((void *)ctx);
}
static void emu_timer_suspend (void)
{
		emu_timer_suspended = TRUE;
		/*@i@*/ while (emu_timer_isticking)
				(void) pthread_yield ();
}
static void emu_timer_resume (void)
{
		emu_timer_suspended = FALSE;
}
/*@null@*/ static void * emu_timer_exec (void * c)
{
		UNUSED (c);
		emu_timer_isrunning = TRUE;
		/*@i@*/ while (emu_timer_isrunning) {
				/*@i@*/ while (emu_timer_suspended)
						(void) pthread_yield ();
				emu_timer_isticking = TRUE;
				emu_timer_tick ();
				emu_timer_isticking = FALSE;
				(void) usleep ((useconds_t) emu_timer_scale);
		}
		return NULL;
}
static boolean emu_timer_init (void)
{
		/*@i@*/ if (pthread_mutex_init (&emu_timer_lock, NULL) != 0)
				return FALSE;
		/*@i@*/ if (pthread_create (&emu_timer_thread, NULL, emu_timer_exec, NULL) != 0)
				return FALSE;
		/*@i@*/ while (!emu_timer_isrunning)
				/*@i@*/ (void) pthread_yield ();
		return TRUE;
}
static boolean emu_timer_term (void)
{
		emu_timer_suspended = FALSE;
		emu_timer_isrunning = FALSE;
		/*@i@*/ (void) pthread_join (emu_timer_thread, NULL);
		(void) pthread_mutex_destroy (&emu_timer_lock);
		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/
/* emu_gps */

/*@null@*/ /*@shared@*/ static emu_timer_t* emu_gps_timectx = NULL;
static char emu_gps_buffer_data [16384];
static long emu_gps_buffer_offs_wr = 0, emu_gps_buffer_offs_rd = 0;
static boolean emu_gps_buffer_eof = FALSE;
/*@null@*/ static FILE* emu_gps_source_file = NULL;
static char emu_gps_tohexchar (unsigned char c)
{
		return (c <= (unsigned char) 9) ? (char) (c + (unsigned char) '0') : (char) (c - (unsigned char) 10 + (unsigned char) 'A');
}
static boolean emu_gps_populate_file (char * buffer, int length)
{
		assert (buffer != NULL && length > 0);
		if (emu_gps_source_file == NULL || feof (emu_gps_source_file) != 0)
				return FALSE;
		(void) fgets (buffer, length, emu_gps_source_file);
		return TRUE;
}
static boolean emu_gps_populate_rand (char * buffer, int length)
{
		static int zero_run = 0;
		assert (buffer != NULL && length > 0);
		if (zero_run == 0 && (rand () % 1000) < 998)
		{
				time_t tc = time (NULL);
				struct tm * t = gmtime (&tc);
				assert (t != NULL);
/*@-evalorder@*/
				(void) snprintf (buffer, (size_t) length, "$GPGGA,%02d%02d%02d,%02d%02d.%04d,%c,%03d%02d.%04d,%c,1,06,1.5,%d.%0d,M,,,,,",
						t->tm_hour, t->tm_min, t->tm_sec,
						rand () % 90, rand () % 60, rand () % 9999, (rand () % 2) > 0 ? 'N' : 'S',
						rand () % 180, rand () % 60, rand () % 9999, (rand () % 2) > 0 ? 'E' : 'W',
						rand () % 1000, rand () % 9);
/*@+evalorder@*/
		}
		else
		{
				if (zero_run-- == 0) zero_run = (int) (rand () % 750);
				(void) strcpy (buffer, "$GPGGA,,,,,,,01,,,,,,,,");
		}
		return TRUE;
}
static void emu_gps_populate_csum (char * buffer, int length)
{
		unsigned char checksum = (unsigned char) 0;
		char * buffer_ptr = buffer;
		assert (buffer != NULL && length > 0);
		UNUSED (length);
		while (*buffer_ptr != '$')
				buffer_ptr++;
		buffer_ptr++;
		while (*buffer_ptr != '\0')
				checksum ^= *buffer_ptr++;
		*buffer_ptr++ = '*';
		*buffer_ptr++ = emu_gps_tohexchar ((checksum >> 4) & 0x0F);
		*buffer_ptr++ = emu_gps_tohexchar (checksum & 0x0F);
		*buffer_ptr++ = '\r';
		*buffer_ptr++ = '\n';
		*buffer_ptr++ = '\0';
}
static boolean emu_gps_populate (char * buffer, int length)
{
		assert (buffer != NULL && length > 0);
		if (emu_gps_source_file != NULL) {
				if (emu_gps_populate_file (buffer, length) == FALSE)
						return FALSE;
		} else {
				if (emu_gps_populate_rand (buffer, length) == FALSE)
						return FALSE;
				if (buffer [0] != '\0')
						emu_gps_populate_csum (buffer, length);
		}
		return TRUE;
}
static void emu_gps_ticker (emu_timer_t* c, long s)
{
		static char buffer [512];
		assert (c != NULL && c == emu_gps_timectx);
		assert (s > 0);
		UNUSED (c);
		UNUSED (s);
		buffer [0] = '\0';
		if (emu_gps_populate (buffer, (int) sizeof (buffer)) == FALSE) {
				if (emu_gps_timectx != NULL)
						emu_timer_destroy (emu_gps_timectx);
				emu_gps_timectx = NULL;
				emu_gps_buffer_eof = TRUE;
				return;
		}
		if (buffer [0] != '\0') {
				long cntr = 0;
				while (buffer [cntr] != '\0') {
						long onxt = (emu_gps_buffer_offs_wr + 1) % sizeof (emu_gps_buffer_data);
						if (onxt == emu_gps_buffer_offs_rd)
								break;
						emu_gps_buffer_data [emu_gps_buffer_offs_wr] = buffer [cntr++];
						emu_gps_buffer_offs_wr = onxt;
				}
		}
}
static void emu_gps_init (void)
{
/*@-evalorderuncon@*/
		emu_gps_timectx = emu_timer_create (emu_gps_ticker, (signed long) 1, TRUE);
/*@=evalorderuncon@*/
		assert (emu_gps_timectx != NULL);
}
static void emu_gps_term (void)
{
		if (emu_gps_timectx != NULL)
				emu_timer_destroy (emu_gps_timectx);
}
static char emu_gps_getchar (void)
{
		char ch;
		/*@i@*/ while (emu_gps_buffer_offs_wr == emu_gps_buffer_offs_rd) {
				if (emu_gps_buffer_eof) {
						lpc21XX_shutdown_trigger ();
						return '\n';
				}
				(void) pthread_yield ();
		}
		ch = emu_gps_buffer_data [emu_gps_buffer_offs_rd++];
		if (emu_gps_buffer_offs_rd == (long) sizeof (emu_gps_buffer_data))
				emu_gps_buffer_offs_rd = 0;
		if (emu_gps_buffer_offs_wr == emu_gps_buffer_offs_rd && emu_gps_buffer_eof)
				lpc21XX_shutdown_trigger ();
		return ch;
}
static unsigned int emu_gps_getstring (/*@notnull@*/ /*@out@*/ unsigned char * const buffer, const unsigned int length)
{
		unsigned int count = 0;
		while (count < length)
		{
				boolean eol = FALSE;

				/*@i@*/ while (emu_gps_buffer_offs_wr == emu_gps_buffer_offs_rd) {
						if (emu_gps_buffer_eof) {
								lpc21XX_shutdown_trigger ();
								return 0;
						}
						(void) pthread_yield ();
				}
				if ((buffer [count++] = (unsigned char) emu_gps_buffer_data [emu_gps_buffer_offs_rd++]) == (unsigned char) '\n')
						eol = TRUE;
				if (emu_gps_buffer_offs_rd == (long) sizeof (emu_gps_buffer_data))
						emu_gps_buffer_offs_rd = 0;
				if (emu_gps_buffer_offs_wr == emu_gps_buffer_offs_rd && emu_gps_buffer_eof)
						lpc21XX_shutdown_trigger ();
				if (eol)
						break;
		}
		return count;
}
static void emu_gps_putchar (const unsigned char ch)
{
		UNUSED (ch);
}
static void emu_gps_write (/*@notnull@*/ const unsigned char * buffer, const unsigned int length)
{
		UNUSED (buffer);
		UNUSED (length);
}

/* ----------------------------------------------------------------------------------------------------*/
/* emu_spi */

/*@null@*/ static char * emu_spi_file = NULL;
static int emu_spi_fd = -1;
/*@-namechecks@*/
#define EMU_SPI_S_COMMAND		0
#define EMU_SPI_S_READSECTOR	1
#define EMU_SPI_S_WRITESECTOR	2
/*@=namechecks@*/
static int emu_spi_state = EMU_SPI_S_COMMAND;
static unsigned char emu_spi_command_data [8];
static int emu_spi_command_size = 0;
static unsigned char emu_spi_response_data [2048];
static int emu_spi_response_size = 0, emu_spi_response_offs = 0;
static unsigned long emu_spi_stat_block_read = 0, emu_spi_stat_block_write = 0;
static void emu_spi_init (void)
{
		if (emu_spi_fd == -1) {
				int lock_status;
				assert (emu_spi_file != NULL);
				emu_spi_fd = open (emu_spi_file, O_RDWR|O_NONBLOCK);
				assert (emu_spi_fd > 0);
				/*@-unrecog@*/ lock_status = flock (emu_spi_fd, LOCK_EX|LOCK_NB); /*@=unrecog@*/
				assert (lock_status == 0);
		}
		emu_spi_state = EMU_SPI_S_COMMAND;
		emu_spi_command_size = emu_spi_response_size = emu_spi_response_offs = 0;
}
static void emu_spi_term (void)
{
		emu_spi_state = EMU_SPI_S_COMMAND;
		emu_spi_command_size = emu_spi_response_size = emu_spi_response_offs = 0;
		/* (void) close (emu_spi_fd); */
}
static unsigned char emu_spi_get (void)
{
		if (emu_spi_response_offs < emu_spi_response_size)
				return emu_spi_response_data [emu_spi_response_offs++];
		return (unsigned char) 0xFF;
}
static void emu_spi_put (const unsigned char ch)
{
		static unsigned long cmd_sector_number = 0;
		static unsigned long cmd_sector_offset = 0;
		static unsigned long cmd_sector_blocksz = 0;
		static unsigned char cmd_sector_buffer [2048];
		long temp;

		if (emu_spi_state == EMU_SPI_S_COMMAND) {
				if (emu_spi_command_size == 0 && ch == (unsigned char) 0xFF)
						return;
				if (emu_spi_command_size < 6) {
						emu_spi_command_data [emu_spi_command_size++] = ch;
						if (emu_spi_command_size == 6) {
								/* execute command */
								if (emu_spi_command_data [0] == (unsigned char) 0x40) { /* CMD0, reset */
										emu_spi_response_data [0] = (unsigned char) 0x01;
										emu_spi_response_size = 1;
										emu_spi_response_offs = 0;
								} else if (emu_spi_command_data [0] == (unsigned char) 0x41) { /* CMD1, idle->active */
										emu_spi_response_data [0] = (unsigned char) 0x00;
										emu_spi_response_size = 1;
										emu_spi_response_offs = 0;
								} else if (emu_spi_command_data [0] == (unsigned char) (unsigned char) 0x50) { /* CMD16, block length */
										emu_spi_response_data [0] = (unsigned char) 0x00;
										emu_spi_response_size = 1;
										emu_spi_response_offs = 0;
										cmd_sector_blocksz = ((unsigned long) (emu_spi_command_data [2] & 0xFF) << 16) |
															((unsigned long) (emu_spi_command_data [3] & 0xFF) << 8) |
															((unsigned long) (emu_spi_command_data [4] & 0xFF));
								} else if (emu_spi_command_data [0] == (unsigned char) 0x51) { /* CMD17, read single block */
										emu_spi_stat_block_read++;
										emu_spi_response_data [0] = (unsigned char) 0x00;
										emu_spi_response_data [1] = (unsigned char) 0xFE;
										emu_spi_response_size = 2;
										emu_spi_response_offs = 0;
										cmd_sector_number = ((unsigned long) (emu_spi_command_data [3] & 0xFF) >> 1) |
															((unsigned long) (emu_spi_command_data [2] & 0xFF) << (8 - 1)) |
															((unsigned long) (emu_spi_command_data [1] & 0xFF) << (16 - 1));
										cmd_sector_offset = 0;
										temp = (long) lseek (emu_spi_fd, (off_t) (cmd_sector_number * cmd_sector_blocksz), SEEK_SET);
										assert (temp == (long) (cmd_sector_number * cmd_sector_blocksz));
										temp = (long) read (emu_spi_fd, (void *) &emu_spi_response_data [emu_spi_response_size], (size_t) cmd_sector_blocksz);
										assert (temp == (long) cmd_sector_blocksz);
										emu_spi_response_size += cmd_sector_blocksz;
								} else if (emu_spi_command_data [0] == (unsigned char) 0x58) { /* CMD24, write single block */
										emu_spi_stat_block_write++;
										emu_spi_response_data [0] = (unsigned char) 0x00;
										emu_spi_response_size = 1;
										emu_spi_response_offs = 0;
										cmd_sector_number = ((unsigned long) (emu_spi_command_data [3] & 0xFF) >> 1) |
															((unsigned long) (emu_spi_command_data [2] & 0xFF) << (8 - 1)) |
															((unsigned long) (emu_spi_command_data [1] & 0xFF) << (16 - 1));
										cmd_sector_offset = (unsigned long) -1;
										temp = (long) lseek (emu_spi_fd, (off_t) (cmd_sector_number * cmd_sector_blocksz), SEEK_SET);
										assert (temp == (long) (cmd_sector_number * cmd_sector_blocksz));
										emu_spi_state = EMU_SPI_S_WRITESECTOR;
								}
								emu_spi_command_size = 0;
						}
				}
		} else if (emu_spi_state == EMU_SPI_S_WRITESECTOR) {
				if (cmd_sector_offset == (unsigned long) -1) {
						assert (ch == (unsigned char) 0xFE);
						cmd_sector_offset++;
				} else {
						cmd_sector_buffer [cmd_sector_offset++] = ch;
						if (cmd_sector_offset == cmd_sector_blocksz) {
								temp = (long) write (emu_spi_fd, (void *)cmd_sector_buffer, (size_t) cmd_sector_blocksz);
								assert (temp == (long) cmd_sector_blocksz);
								emu_spi_response_data [0] = (unsigned char) 0x05;
								emu_spi_response_size = 1;
								emu_spi_response_offs = 0;
								emu_spi_state = EMU_SPI_S_COMMAND;
						}
				}
		}
}

/* ----------------------------------------------------------------------------------------------------*/
/* emu_**** */

static void emu_down (void) __attribute__ ((noreturn));
static void emu_halt (void) __attribute__ ((noreturn));
static void emu_reset (void) __attribute__ ((noreturn));

static void emu_idle (void)
{
		fprintf (stderr, ".");
		(void) pthread_yield ();
}

static void emu_down (void)
{
		fprintf (stderr, "\nDOWN\n");
		exit (EXIT_SUCCESS);
}

static void emu_halt (void)
{
		(void) fflush (stdout);
		fprintf (stderr, "\nHALT\n");
		abort ();
}

static void emu_reset (void)
{
		fprintf (stderr, "\nRESET\n");
		exit (EXIT_SUCCESS);
}

/* ----------------------------------------------------------------------------------------------------*/
/* emu_file */

/*@null@*/ char * emu_file_dir = NULL;


/* ----------------------------------------------------------------------------------------------------*/
/* emu_init */

static boolean emu_exists_file (/*@notnull@*/ const char * const file)
{
		struct stat sb;
		return (stat (file, &sb) == 0 && S_ISREG (sb.st_mode)) ? TRUE : FALSE;
}
static boolean emu_exists_dirt (/*@notnull@*/ const char * const dirt)
{
		struct stat sb;
		return (stat (dirt, &sb) == 0 && S_ISDIR (sb.st_mode)) ? TRUE : FALSE;
}

static void emu_usage (const char * const name)
{
		fprintf (stderr, "usage: %s [-v] [-h] [-i fat_image|-d file_dir] [-f gps_source] [-s timer_scale] [--] "
						 "[--cfg-nowrite] [--cfg_<name>=<value>]\n", name);
}
static boolean emu_init (const int argc, const char ** const argv)
{
		int ch;

		fprintf (stderr, "gps_logger_mg v" VERSION ": firmware emulator (platform: " SYSTEM ")\n");
		fprintf (stderr, COPYRIGHT "\n");

/*@-loopswitchbreak@*/
		while ((ch = getopt (argc, (char ** const) argv, "i:d:f:s:hv")) != -1) {
				switch (ch) {
				case 'i':
						/*@i@*/ emu_spi_file = strdup (optarg);
						if (emu_spi_file == NULL || !emu_exists_file (emu_spi_file)) {
								fprintf (stderr, "fatal: cannot open sd-fat16 image '%s'\n", optarg);
								exit (EXIT_FAILURE);
						}
						break;
				case 'd':
						/*@i@*/ emu_file_dir = strdup (optarg);
						if (emu_file_dir == NULL || !emu_exists_dirt (emu_file_dir)) {
								fprintf (stderr, "fatal: cannot open sd-fat16 directory '%s'\n", optarg);
								exit (EXIT_FAILURE);
						}
						break;
				case 'f':
						emu_gps_source_file = strcmp (optarg, "-") == 0 ? stdin : fopen (optarg, "r");
						if (emu_gps_source_file == NULL) {
								fprintf (stderr, "fatal: cannot open source file '%s'\n", optarg);
								exit (EXIT_FAILURE);
						}
						break;
				case 's':
						emu_timer_scale = (unsigned long) atol (optarg);
						break;
				default:
				case 'h':
						emu_usage (argv [0]);
						exit (EXIT_SUCCESS);
				case 'v':
						exit (EXIT_SUCCESS);
				}
		}
/*@=loopswitchbreak@*/

		if (emu_spi_file == NULL && emu_file_dir == NULL) {
				emu_usage (argv [0]);
				exit (EXIT_FAILURE);
		}

		srand ((unsigned int) (time (NULL) ^ getpid ()));

		if (emu_timer_init () == FALSE)
				return FALSE;

		return TRUE;
}
static boolean emu_term (void)
{
		if (emu_spi_file != NULL) {
				DPRINTF (("emu_spi_stat: block_read=%lu, block_write=%lu\n", emu_spi_stat_block_read, emu_spi_stat_block_write));
		}

		(void) emu_timer_term ();

		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/
/* ----------------------------------------------------------------------------------------------------*/

static boolean lpc21XX_init_flag = FALSE;

boolean lpc21XX_init (const int argc, const char ** const argv)
{
		assert (!lpc21XX_init_flag);
		lpc21XX_init_flag = TRUE;

		if (!emu_init (argc, argv))
				return FALSE;

		DPRINTF (("lpc21XX_init\n"));

		lpc21XX_led_init ();

		return TRUE;
}

boolean lpc21XX_term (void)
{
		assert (lpc21XX_init_flag);
		lpc21XX_init_flag = FALSE;

		DPRINTF (("lpc21XX_term\n"));

		(void) emu_term ();

		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

void lpc21XX_delay (const unsigned int count)
{
		unsigned int i;

		for (i = 0; i < count; i++)
		{
				;
		}
}

/* ----------------------------------------------------------------------------------------------------*/

void lpc21XX_idle (void)
{
		emu_idle ();
}

void lpc21XX_down (void)
{
		lpc21XX_delay ((unsigned int) 64);

		(void) lpc21XX_term ();

		emu_down ();
}

void lpc21XX_halt (void)
{
		lpc21XX_delay ((unsigned int) 64);

		(void) lpc21XX_term ();

		emu_halt ();
}

void lpc21XX_reset (void)
{
		lpc21XX_delay ((unsigned int) 64);

		(void) lpc21XX_term ();

		emu_reset ();
}

/* ----------------------------------------------------------------------------------------------------*/

static time_t lpc21XX_rtc_timetck = 0;
/*@shared@*/ /*@null@*/ static emu_timer_t* lpc21XX_rtc_timectx = NULL;
static boolean lpc21XX_rtc_enabled_flag = FALSE;

static void emu_lpc21XX_rtc_tick (emu_timer_t* c, long s)
{
		assert (c != NULL && c == lpc21XX_rtc_timectx);
		assert (s > 0);

		UNUSED (c);
		UNUSED (s);

		lpc21XX_rtc_timetck++;
}

boolean lpc21XX_rtc_enabled (void)
{
		return lpc21XX_rtc_enabled_flag;
}

void lpc21XX_rtc_enable (void)
{
		assert (!lpc21XX_rtc_enabled_flag);
		lpc21XX_rtc_enabled_flag = TRUE;

/*@-evalorderuncon@*/
		lpc21XX_rtc_timectx = emu_timer_create (emu_lpc21XX_rtc_tick, (signed long) 1, TRUE);
/*@=evalorderuncon@*/
		assert (lpc21XX_rtc_timectx != NULL);
}

void lpc21XX_rtc_disable (void)
{
		assert (lpc21XX_rtc_enabled_flag);
		lpc21XX_rtc_enabled_flag = FALSE;

		assert (lpc21XX_rtc_timectx != NULL);
		emu_timer_destroy (lpc21XX_rtc_timectx);
}

void lpc21XX_rtc_get (lpc21XX_rtc_t * const c)
{
		if (lpc21XX_rtc_enabled_flag)
		{
			struct tm * t = localtime (&lpc21XX_rtc_timetck);
			assert (c != NULL);
			assert (t != NULL);
			c->e [LPC21XX_RTC_ELEM_YY] = (unsigned char) (t->tm_year + 1900 - 1980);
			c->e [LPC21XX_RTC_ELEM_MO] = (unsigned char) (t->tm_mon + 1);
			c->e [LPC21XX_RTC_ELEM_DD] = (unsigned char) t->tm_mday;
			c->e [LPC21XX_RTC_ELEM_HH] = (unsigned char) t->tm_hour;
			c->e [LPC21XX_RTC_ELEM_MM] = (unsigned char) t->tm_min;
			c->e [LPC21XX_RTC_ELEM_SS] = (unsigned char) t->tm_sec;
		}
		else
		{
			c->e [LPC21XX_RTC_ELEM_YY] = (unsigned char) 0;
			c->e [LPC21XX_RTC_ELEM_MO] = (unsigned char) 1;
			c->e [LPC21XX_RTC_ELEM_DD] = (unsigned char) 1;
			c->e [LPC21XX_RTC_ELEM_HH] = (unsigned char) 0;
			c->e [LPC21XX_RTC_ELEM_MM] = (unsigned char) 0;
			c->e [LPC21XX_RTC_ELEM_SS] = (unsigned char) 0;
		}
}

void lpc21XX_rtc_set (const lpc21XX_rtc_t * const c)
{
		assert (lpc21XX_rtc_enabled_flag);

		{
			struct tm t;
			memset ((void *)&t, 0, sizeof (t));
			assert (c != NULL);
			assert (c->e [LPC21XX_RTC_ELEM_YY] < (unsigned char) 90 && c->e [LPC21XX_RTC_ELEM_MO] <= (unsigned char) 12 && c->e [LPC21XX_RTC_ELEM_DD] <= (unsigned char) 31 &&
					c->e [LPC21XX_RTC_ELEM_HH] < (unsigned char) 24 && c->e [LPC21XX_RTC_ELEM_MM] < (unsigned char) 60 && c->e [LPC21XX_RTC_ELEM_SS] < (unsigned char) 60);
			t.tm_year = (int) ((int) c->e [LPC21XX_RTC_ELEM_YY] + (int) 1980 - (int) 1900);
			t.tm_mon = (int) (c->e [LPC21XX_RTC_ELEM_MO] - (unsigned char) 1);
			t.tm_mday = (int) c->e [LPC21XX_RTC_ELEM_DD];
			t.tm_hour = (int) c->e [LPC21XX_RTC_ELEM_HH];
			t.tm_min = (int) c->e [LPC21XX_RTC_ELEM_MM];
			t.tm_sec = (int) c->e [LPC21XX_RTC_ELEM_SS];
			lpc21XX_rtc_timetck = mktime (&t);
		}
}

/* ----------------------------------------------------------------------------------------------------*/

static boolean lpc21XX_led_enabled_flag = FALSE;

#ifdef HARDWARE_10
#define lpc21XX_led_cfg_size ((unsigned char) 2)
#endif
#ifdef HARDWARE_24
#define lpc21XX_led_cfg_size ((unsigned char) 3)
#endif

void lpc21XX_led_enable (const unsigned char led)
{
		assert (led <= lpc21XX_led_cfg_size);

		assert (lpc21XX_led_enabled_flag);

		UNUSED (led);
}

void lpc21XX_led_disable (const unsigned char led)
{
		assert (led <= lpc21XX_led_cfg_size);

		assert (lpc21XX_led_enabled_flag);

		UNUSED (led);
}

void lpc21XX_led_toggle (const unsigned char led)
{
		assert (led <= lpc21XX_led_cfg_size);

		assert (lpc21XX_led_enabled_flag);

		UNUSED (led);
}

unsigned char lpc21XX_led_size (void)
{
		assert (lpc21XX_led_enabled_flag);

		return lpc21XX_led_cfg_size;
}

void lpc21XX_led_init (void)
{
		assert (!lpc21XX_led_enabled_flag);
		lpc21XX_led_enabled_flag = TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

static const unsigned int lpc21XX_uart_baud_cfg_list [] =
{
		(unsigned int)	1200, (unsigned int)  2400, (unsigned int)	4800, (unsigned int)  9600,
		(unsigned int) 19200, (unsigned int) 38400, (unsigned int) 57600, (unsigned int)115200
};
#define lpc21XX_uart_baud_cfg_size ((int) (sizeof (lpc21XX_uart_baud_cfg_list) / sizeof (lpc21XX_uart_baud_cfg_list [0])))

boolean lpc21XX_uart_baud_supported (const unsigned int baud_rate)
{
		int i;

		for (i = 0; i < lpc21XX_uart_baud_cfg_size; i++)
		{
				if (lpc21XX_uart_baud_cfg_list [i] == baud_rate)
				{
						return TRUE;
				}
		}

		return FALSE;
}

/* ----------------------------------------------------------------------------------------------------*/

static boolean lpc21XX_uart0_init_flag = FALSE;

void lpc21XX_uart0_init (const unsigned int baud_rate, const boolean enable_interrupts)
{
		boolean okay = lpc21XX_uart_baud_supported (baud_rate);

		assert (okay);
		assert (baud_rate > 0);
		assert (enable_interrupts || !enable_interrupts);

		assert (!lpc21XX_uart0_init_flag);
		lpc21XX_uart0_init_flag = TRUE;

		UNUSED (baud_rate);
		UNUSED (enable_interrupts);
		UNUSED (okay);

		(void) setvbuf (stdout, NULL, (int) _IONBF, 0);
}

void lpc21XX_uart0_term (void)
{
		assert (lpc21XX_uart0_init_flag);
		lpc21XX_uart0_init_flag = FALSE;

		(void) fflush (stdout);
}

void lpc21XX_uart0_set_fifo_level (const unsigned char value)
{
		assert (lpc21XX_uart0_init_flag);

		UNUSED (value);
}

void lpc21XX_uart0_putc (const unsigned char ch)
{
		assert (lpc21XX_uart0_init_flag);

		(void) fputc ((int) ch, stdout);
}

unsigned char lpc21XX_uart0_getc (void)
{
		assert (lpc21XX_uart0_init_flag);

		return (unsigned char) fgetc (stdin);
}

unsigned int lpc21XX_uart0_gets (unsigned char * const buffer, const unsigned int length)
{
		unsigned int count = 0;
		assert (lpc21XX_uart0_init_flag);

/*@-usedef@*/
		if (fgets ((char *) buffer, (int) length, stdin) != NULL)
				while (count < length && buffer [count] != (unsigned char) '\0')
						count++;
/*@=usedef@*/
		return count;
}

void lpc21XX_uart0_write (const unsigned char * const buffer, const unsigned int length)
{
		assert (lpc21XX_uart0_init_flag);

		(void) fwrite ((const void *) buffer, (size_t) 1, (size_t) length, stdout);
}

/* ----------------------------------------------------------------------------------------------------*/

static boolean lpc21XX_uart1_init_flag = FALSE;

void lpc21XX_uart1_init (const unsigned int baud_rate, const boolean enable_interrupts)
{
		boolean okay = lpc21XX_uart_baud_supported (baud_rate);

		assert (okay);
		assert (baud_rate > 0);
		assert (enable_interrupts || !enable_interrupts);

		assert (!lpc21XX_uart1_init_flag);
		lpc21XX_uart1_init_flag = TRUE;

		UNUSED (baud_rate);
		UNUSED (enable_interrupts);
		UNUSED (okay);

		emu_gps_init ();
}

void lpc21XX_uart1_term (void)
{
		assert (lpc21XX_uart1_init_flag);
		lpc21XX_uart1_init_flag = FALSE;

		emu_gps_term ();
}

void lpc21XX_uart1_set_fifo_level (const unsigned char value)
{
		assert (lpc21XX_uart1_init_flag);

		UNUSED (value);
}

void lpc21XX_uart1_putc (const unsigned char ch)
{
		assert (lpc21XX_uart1_init_flag);

		emu_gps_putchar (ch);
}

unsigned char lpc21XX_uart1_getc (void)
{
		assert (lpc21XX_uart1_init_flag);

		return (unsigned char) emu_gps_getchar ();
}

unsigned int lpc21XX_uart1_gets (unsigned char * const buffer, const unsigned int length)
{
		assert (lpc21XX_uart1_init_flag);

		return (unsigned int) emu_gps_getstring (buffer, length);
}

void lpc21XX_uart1_write (const unsigned char * const buffer, const unsigned int length)
{
		assert (lpc21XX_uart1_init_flag);

		(void) emu_gps_write (buffer, length);
}

/* ----------------------------------------------------------------------------------------------------*/

static boolean lpc21XX_spi_init_flag = FALSE;

void lpc21XX_spi_init (void)
{
		assert (!lpc21XX_spi_init_flag);
		lpc21XX_spi_init_flag = TRUE;

		emu_spi_init ();
}

void lpc21XX_spi_term (void)
{
		assert (lpc21XX_spi_init_flag);
		lpc21XX_spi_init_flag = FALSE;

		emu_spi_term ();
}

void lpc21XX_spi_put (const unsigned char ch)
{
		assert (lpc21XX_spi_init_flag);

		emu_spi_put (ch);
}

unsigned char lpc21XX_spi_get (void)
{
		assert (lpc21XX_spi_init_flag);

		return emu_spi_get ();
}

void lpc21XX_spi_acquire (void)
{
		assert (lpc21XX_spi_init_flag);
}

void lpc21XX_spi_release (void)
{
		assert (lpc21XX_spi_init_flag);
}

/* ----------------------------------------------------------------------------------------------------*/

static boolean lpc21XX_timer0_init_flag = FALSE;
/*@null@*/ /*@shared@*/ static emu_timer_t * lpc21XX_timer0_timectx = NULL;
static unsigned long lpc21XX_timer0_secs;
/*@null@*/ static lpc21XX_timer_callback_t lpc21XX_timer0_callback;

static void emu_lpc21XX_timer0_trigger (emu_timer_t* t, long s)
{
		assert (t != NULL && t == lpc21XX_timer0_timectx);
		assert (s > 0);

		UNUSED (t);
		UNUSED (s);

		if (lpc21XX_timer0_callback != NULL)
		{
				(*lpc21XX_timer0_callback) ();
		}
}

void lpc21XX_timer0_suspend (void)
{
		emu_timer_suspend ();
}

void lpc21XX_timer0_resume (void)
{
		emu_timer_resume ();
}

void lpc21XX_timer0_init (const lpc21XX_timer_callback_t callback)
{
		assert (callback != NULL);

		assert (!lpc21XX_timer0_init_flag);
		lpc21XX_timer0_init_flag = TRUE;

		lpc21XX_timer0_callback = callback;
}

void lpc21XX_timer0_term (void)
{
		assert (lpc21XX_timer0_init_flag);
		lpc21XX_timer0_init_flag = FALSE;

		if (lpc21XX_timer0_timectx != NULL) {
				emu_timer_destroy (lpc21XX_timer0_timectx);
				lpc21XX_timer0_timectx = NULL;
		}
}

void lpc21XX_timer0_start (const unsigned long secs)
{
		assert (secs > 0);

		assert (lpc21XX_timer0_init_flag);

		lpc21XX_timer0_secs = secs;

		if (lpc21XX_timer0_timectx != NULL)
				emu_timer_destroy (lpc21XX_timer0_timectx);
/*@-evalorderuncon@*/
		lpc21XX_timer0_timectx = emu_timer_create (emu_lpc21XX_timer0_trigger, (signed long) lpc21XX_timer0_secs, FALSE);
/*@=evalorderuncon@*/
		assert (lpc21XX_timer0_timectx != NULL);
}

void lpc21XX_timer0_stop (void)
{
		assert (lpc21XX_timer0_init_flag);

		if (lpc21XX_timer0_timectx != NULL) {
				emu_timer_destroy (lpc21XX_timer0_timectx);
				lpc21XX_timer0_timectx = NULL;
		}	
}

/* ----------------------------------------------------------------------------------------------------*/

static boolean lpc21XX_powerfail_init_flag = FALSE;
/*@null@*/ static lpc21XX_powerfail_callback_t lpc21XX_powerfail_callback;

/*static void lpc21XX_powerfail_irq_handler (void)
{
		if (lpc21XX_powerfail_callback != NULL)
		{
				(*lpc21XX_powerfail_callback) ();
		}
}*/

void lpc21XX_powerfail_enable (const lpc21XX_powerfail_callback_t callback)
{
		assert (callback != NULL);

		assert (!lpc21XX_powerfail_init_flag);
		lpc21XX_powerfail_init_flag = TRUE;

		lpc21XX_powerfail_callback = callback;
}

void lpc21XX_powerfail_disable (void)
{
		assert (lpc21XX_powerfail_init_flag);
		lpc21XX_powerfail_init_flag = FALSE;

		lpc21XX_powerfail_callback = NULL;
}

/* ----------------------------------------------------------------------------------------------------*/

static boolean lpc21XX_watchdog_enable_flag = FALSE;
/*@shared@*/ /*@null@*/ static emu_timer_t* lpc21XX_watchdog_timectx = NULL;
static unsigned long lpc21XX_watchdog_timeout = 0;
static unsigned long lpc21XX_watchdog_counter = 0;

static void emu_lpc21XX_watchdog_trigger (emu_timer_t* t, long s)
{
		assert (t != NULL && t == lpc21XX_watchdog_timectx);
		assert (s > 0);

		UNUSED (t);
		UNUSED (s);

		if (lpc21XX_watchdog_counter++ > lpc21XX_watchdog_timeout)
				HALT (("watchdog: expired after %ld secs\n", s));
}

void lpc21XX_watchdog_enable (const unsigned long secs)
{
		assert (secs > (unsigned long) 0);

		assert (!lpc21XX_watchdog_enable_flag);
		lpc21XX_watchdog_enable_flag = TRUE;

		lpc21XX_watchdog_counter = 0;
		lpc21XX_watchdog_timeout = secs;
/*@-evalorderuncon@*/
		lpc21XX_watchdog_timectx = emu_timer_create (emu_lpc21XX_watchdog_trigger, (signed long) 1, TRUE);
/*@=evalorderuncon@*/
		assert (lpc21XX_watchdog_timectx != NULL);
}

void lpc21XX_watchdog_disable (void)
{
		assert (lpc21XX_watchdog_enable_flag);
		lpc21XX_watchdog_enable_flag = FALSE;

		assert (lpc21XX_watchdog_timectx != NULL);
		emu_timer_destroy (lpc21XX_watchdog_timectx);
}

void lpc21XX_watchdog_kick (void)
{
		assert (lpc21XX_watchdog_enable_flag);

		lpc21XX_watchdog_counter = 0;
}

/* ----------------------------------------------------------------------------------------------------*/

static boolean lpc21XX_shutdown_init_flag = FALSE;
/*@null@*/ static lpc21XX_shutdown_callback_t lpc21XX_shutdown_callback = NULL;

static void lpc21XX_shutdown_sig_handler (int sig)
{
		UNUSED (sig);
		fprintf (stderr, "\nInterrupted\n");
		lpc21XX_shutdown_trigger ();
		(void) signal (SIGINT, SIG_IGN);
		(void) signal (SIGTERM, SIG_IGN);
}

void lpc21XX_shutdown_trigger (void)
{
		if (lpc21XX_shutdown_callback != NULL)
		{
				(*lpc21XX_shutdown_callback) ();
		}
}

void lpc21XX_shutdown_enable (const lpc21XX_shutdown_callback_t callback)
{
		assert (callback != NULL);

		assert (!lpc21XX_shutdown_init_flag);
		lpc21XX_shutdown_init_flag = TRUE;

		lpc21XX_shutdown_callback = callback;

		(void) signal (SIGINT, lpc21XX_shutdown_sig_handler);
		(void) signal (SIGTERM, lpc21XX_shutdown_sig_handler);
}

void lpc21XX_shutdown_disable (void)
{
		assert (lpc21XX_shutdown_init_flag);
		lpc21XX_shutdown_init_flag = FALSE;

		(void) signal (SIGINT, SIG_IGN);
		(void) signal (SIGTERM, SIG_IGN);

		lpc21XX_shutdown_callback = NULL;
}

/* ----------------------------------------------------------------------------------------------------*/

static boolean lpc21XX_gpshw_enable_flag = FALSE;

void lpc21XX_gpshw_enable (void)
{
		assert (!lpc21XX_gpshw_enable_flag);
		lpc21XX_gpshw_enable_flag = TRUE;
}

void lpc21XX_gpshw_disable (void)
{
		assert (lpc21XX_gpshw_enable_flag);
		lpc21XX_gpshw_enable_flag = FALSE;
}

/* ----------------------------------------------------------------------------------------------------*/

#ifdef TEST_ENABLED

test_result_t lpc21XX_test (void)
{
		return TEST_RESULT_OKAY;
}

#endif

/* ----------------------------------------------------------------------------------------------------*/
