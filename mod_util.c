
/* ----------------------------------------------------------------------------------------------------*/
/*
	mod_util.c: gps logger module for utility functions.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#include "common.h"
#include "mod_lpc21XXhal.h"
#include "mod_util.h"

#ifdef BUILTIN_PRINTF
#include <stdio.h> /* for vprintf */
#include <stdarg.h> /* for va_list */
#endif

#ifndef INLINE
#ifndef BUILTIN_MEMOPS
#include <string.h> /* for mem* */
#endif
#endif

/* ----------------------------------------------------------------------------------------------------*/

unsigned int util_stratol (char * const string, const signed long integer)
{
		signed long value = integer;
		unsigned int offset = 0;
		char buffer [12];
		unsigned int iodex = (unsigned int) sizeof (buffer);

		assert (string != NULL);

		if (value < 0)
		{
				string [offset++] = '-';
				value = -value;
		}

		do
		{
				buffer [--iodex] = (char) (value % (signed long) 10) + '0';
				value /= (signed long) 10;
		}
		while (value != 0);

		do
		{
				string [offset++] = buffer [iodex++];
		}
		while (iodex < (unsigned int) sizeof (buffer));

		string [offset] = '\0';

		return offset;
}

unsigned int util_strcat (char * const string, const char * const content)
{
		char * string_ptr = string;
		const char * content_ptr = content;

		assert (string != NULL);
		assert (content != NULL);

		while (*string_ptr != '\0')
				string_ptr++;

		while ((*string_ptr++ = *content_ptr++) != '\0')
				/*@i@*/ ;

		return ((unsigned int) (string_ptr - string) - (unsigned int) 1);
}

unsigned int util_strpad (char * const string, const char * const content, const char pad, const unsigned int len)
{
		char * string_ptr = string;
		const char * content_ptr = content;

		assert (string != NULL);
		assert (content != NULL);

		while (*content_ptr != '\0')
				content_ptr++;
		if (len > (unsigned int) (content_ptr - content)) {
				unsigned int off = len - (unsigned int) (content_ptr - content);
				while (off-- > 0)
						*string_ptr++ = pad;
		}
		content_ptr = content;
		while ((*string_ptr++ = *content_ptr++) != '\0')
				/*@i@*/ ;

		return ((unsigned int) (string_ptr - string) - (unsigned int) 1);
}

unsigned int util_strcpy (char * const string, const char * const content)
{
		char * string_ptr = string;
		const char * content_ptr = content;

		assert (string != NULL);
		assert (content != NULL);

		while ((*string_ptr++ = *content_ptr++) != '\0')
				/*@i@*/ ;

		return ((unsigned int) (string_ptr - string) - (unsigned int) 1);
}

char * util_strend (char * const string)
{
		char * string_ptr = string;

		assert (string != NULL);

		while (*string_ptr != '\0')
				string_ptr++;

		return string_ptr;
}

unsigned int util_strlen (const char * const content)
{
		const char * content_ptr = content;

		assert (content != NULL);

		while (*content_ptr != '\0')
				content_ptr++;

		return (unsigned int) (content_ptr - content);
}

signed long util_strtol (const char * const content)
{
		boolean neg = FALSE;
		signed long r = 0;
		const char * c = content;

		assert (content != NULL);

		while (*c != '\0' && UTIL_ISSPACE (*c))
				c++;
		if (*c == '-')
				neg = TRUE, c++;
		else if (*c == '+')
				c++;

		while (*c != '\0' && UTIL_ISDIGIT (*c))
				r = (r * 10L) + (signed long) (*c++ - '0');

		return neg ? -r : r;
}

boolean util_strcmpi (const char * const namea, const char * const nameb)
{
		const char * namea_ptr = namea;
		const char * nameb_ptr = nameb;

		assert (namea != NULL);
		assert (nameb != NULL);

		while (*namea_ptr != '\0' && *nameb_ptr != '\0' && UTIL_TOUPPER (*namea_ptr) == UTIL_TOUPPER (*nameb_ptr))
		{
				namea_ptr++;
				nameb_ptr++;
		}

		return (*namea_ptr != '\0' || *nameb_ptr != '\0') ? FALSE : TRUE;
}

boolean util_strcmpix (const char * const name, const char * const list, const char sep)
{
		const char * name_ptr = name;
		const char * list_ptr = list;

		assert (name != NULL);
		assert (list != NULL);

		while (*list_ptr != '\0')
		{
				boolean r = (UTIL_TOUPPER (*name_ptr) != UTIL_TOUPPER (*list_ptr)); /* !matched */

				name_ptr++;
				list_ptr++;

				if (r)
				{
						while (!(*list_ptr == '\0' || *list_ptr == sep))
						{
								list_ptr++;
						}
				}
				else if (*name_ptr == '\0' && (*list_ptr == '\0' || *list_ptr == sep)) /* implicit && matched */
				{
						return TRUE;
				}
				if (*list_ptr == sep)
				{
						list_ptr++;
						name_ptr = name;
				}
		}

		return FALSE;
}

/* ----------------------------------------------------------------------------------------------------*/

#ifndef INLINE
INLINE_DECL void * util_memcpy (void * const dst, const void * const src, const unsigned int len)
{
#ifdef BUILTIN_MEMOPS
		unsigned int n = len;
		unsigned char * d = (unsigned char *) dst;
		const unsigned char * s = (const unsigned char *) src;
#endif

		assert (dst != NULL);
		assert (src != NULL);

#ifdef BUILTIN_MEMOPS
		while (n-- > 0)
				*d++ = *s++;
		/*@i@*/ return dst;
#else
		return memcpy (dst, src, (size_t) len);
#endif
}
#endif

#ifndef INLINE
INLINE_DECL int util_memcmp (const void * const dst, const void * const src, const unsigned int len)
{
#ifdef BUILTIN_MEMOPS
		const unsigned char * d = (const unsigned char *) dst;
		const unsigned char * s = (const unsigned char *) src;
		const unsigned char * e = d + len;
#endif

		assert (dst != NULL);
		assert (src != NULL);

#ifdef BUILTIN_MEMOPS
		while (d < e)
				if (*d++ != *s++)
						return (int) (*--d - *--s);
		return (int) 0;
#else
		return memcmp (dst, src, (size_t) len);
#endif
}
#endif

#ifndef INLINE
INLINE_DECL void * util_memset (void * const dst, const unsigned char chr, const unsigned int len)
{
#ifdef BUILTIN_MEMOPS
		unsigned char * d = (unsigned char *) dst;
		unsigned char * e = d + len;
#endif

		assert (dst != NULL);

#ifdef BUILTIN_MEMOPS
		while (d < e)
				*d++ = chr;
		/*@i@*/ return dst;
#else
		return memset (dst, (int) chr, (size_t) len);
#endif
}
#endif

/* ----------------------------------------------------------------------------------------------------*/

unsigned int util_binpack (unsigned char * const data, const unsigned int offs, const unsigned long integer, const unsigned int nbits)
{
		unsigned int cbits = nbits;
		unsigned int coffs = (offs >> 3), boffs = (offs & 0x07);

		assert (data != NULL);
		assert (nbits <= (unsigned int) 32);

		if (boffs > 0)
		{
				unsigned int xbits = (unsigned int) (8 - boffs);
				data [coffs] &= (unsigned char) (((1 << boffs) - 1) << xbits);
				if (nbits > xbits) {
						data [coffs] |= (unsigned char) ((integer >> (nbits - xbits)) & ((1 << xbits) - 1));
						coffs ++;
						cbits -= xbits;
						boffs = 0;
				} else {
						data [coffs] |= (unsigned char) ((integer << (xbits - nbits)) & ((1 << xbits) - 1));
						cbits = 0;
						boffs = (boffs + nbits) & 0x07;
						if (boffs == 0)
								coffs++;
				}
		}
		while (cbits >= (unsigned int) 8)
		{
				cbits -= 8;
				data [coffs++] = (unsigned char) (integer >> cbits) & 0xFF;
		}
		if (cbits > (unsigned int) 0)
		{
				unsigned int xbits = (unsigned int) (8 - cbits);
				data [coffs] = (unsigned char) (((integer) & ((1 << cbits) - 1)) << xbits);
				boffs = cbits;
		}

		return (coffs << 3) + boffs;
}

/* ----------------------------------------------------------------------------------------------------*/

char * util_makenumberedfn (char * const string, const char * const prefix, const char * const suffix, const unsigned int count)
{
		unsigned int length, offset = 0, ccount;

		assert (string != NULL);
		assert (prefix != NULL);
		assert (suffix != NULL);

		for (length = 0; prefix [length] != '\0' && offset < (unsigned int) 8; length++)
		{
				string [offset++] = prefix [length];
		}

		for (length = (unsigned int) 0, ccount = count; length < (unsigned int) (8 - offset); length++, ccount /= 10)
		{
				string [7 - length] = (char) (ccount % 10) + '0';
		}

		string [8] = '.';

		for (length = (unsigned int) 0; suffix [length] != '\0' && length < (unsigned int) 3; length++)
		{
				string [9 + length] = suffix [length];
		}

		string [9 + length] = '\0';

		return string;
}

/* ----------------------------------------------------------------------------------------------------*/

/*@shared@*/ static unsigned char util_buffer_data [UTIL_BUFFER_SIZE];
static unsigned int util_buffer_offs = 0;

unsigned char * util_buffer_alloc (const unsigned int size)
{
		unsigned char * pntr;

		if ((util_buffer_offs + size) > (unsigned int) UTIL_BUFFER_SIZE)
		{
				return NULL;
		}

		pntr = &util_buffer_data [util_buffer_offs];
		util_buffer_offs += size;
		return pntr;
}

void util_buffer_reset (void)
{
		util_buffer_offs = 0;
}

unsigned char * util_buffer_get (void)
{
		return util_buffer_data;
}

unsigned int util_buffer_len (void)
{
		return (unsigned int) UTIL_BUFFER_SIZE;
}

/* ----------------------------------------------------------------------------------------------------*/

const char * util_csv_field_extract (const char * const input, char * const output, const unsigned char size)
{
		const char * i_ptr = input;
		char * o_ptr = output;
		unsigned int offs = (unsigned int) size - 1;

		assert (input != NULL);
		assert (output != NULL);
		assert (size > (unsigned char) 0);

		if (*i_ptr == '\0')
		{
				return NULL;
		}

		while (offs-- > 0 && *i_ptr != '\0' && *i_ptr != ',')
		{
				*o_ptr++ = *i_ptr++;
		}

		*o_ptr = (char) '\0';

		return (*i_ptr == ',') ? (&i_ptr [1]) : i_ptr;
}

const char * util_csv_field_next (const char * const input)
{
		const char * i_ptr = input;

		assert (input != NULL);

		if (*i_ptr == '\0')
		{
				return NULL;
		}

		while (*i_ptr != '\0' && *i_ptr != ',')
		{
				i_ptr++;
		}

		return (*i_ptr == ',') ? (&i_ptr [1]) : i_ptr;
}

/* ----------------------------------------------------------------------------------------------------*/

#define UTIL_TIMER_SIZE 5

typedef struct st_util_timer_t
{
		/*@shared@*/ util_timer_callback_t callback;
		/*@null@*/ /*@shared@*/ void * calltoken;
		unsigned long secs;
		unsigned long x_rsecs;
		/*@null@*/ /*@shared@*/ struct st_util_timer_t* x_next;
}
util_timer_t;

static boolean util_timer_init = FALSE;
static util_timer_t util_timer_list [UTIL_TIMER_SIZE];
/*@null@*/ static util_timer_t* util_timer_head = NULL;

static void util_timer_expiry (void)
{
		if (util_timer_head != NULL)
		{
				util_timer_head->x_rsecs = 0;
		}

		while (util_timer_head != NULL && util_timer_head->x_rsecs == 0)
		{
				util_timer_t * ctx = util_timer_head;
				util_timer_head = util_timer_head->x_next;
				assert (ctx != NULL);
				(*ctx->callback) (ctx->calltoken);
		}

		if (util_timer_head != NULL)
		{
				lpc21XX_timer0_start (util_timer_head->x_rsecs);
		}
}

static void util_timer_enqueue (/*@shared@*/ /*@notnull@*/ util_timer_t * ctx)
{
		util_timer_t * qp = NULL, * qc = util_timer_head, * qh = util_timer_head;

		assert (ctx != NULL);

		ctx->x_rsecs = ctx->secs;

		while (qc != NULL && qc->x_rsecs < ctx->x_rsecs)
		{
				ctx->x_rsecs -= qc->x_rsecs;
				qp = qc;
				qc = qc->x_next;
		}

		if (qp == NULL)
		{
				util_timer_head = ctx;
		}
		else
		{
				qp->x_next = ctx;
		}
		ctx->x_next = qc;
		if (ctx->x_next != NULL)
		{
				ctx->x_next->x_rsecs -= ctx->x_rsecs;
		}

		if (util_timer_head != NULL && util_timer_head != qh)
		{
				lpc21XX_timer0_start (util_timer_head->x_rsecs);
		}
}

static void util_timer_dequeue (/*@shared@*/ /*@notnull@*/ util_timer_t * ctx)
{
		util_timer_t * qp = NULL, * qc = util_timer_head, * qh = util_timer_head;

		assert (ctx != NULL);

		while (qc != NULL && qc != ctx)
		{
				qp = qc;
				qc = qc->x_next;
		}
		if (qc == NULL)
		{
				return;
		}

		if (qp == NULL)
		{
				util_timer_head = ctx->x_next;
		}
		else
		{
				qp->x_next = ctx->x_next;
		}
		if (ctx->x_next != NULL)
		{
				ctx->x_next->x_rsecs += ctx->x_rsecs;
		}

		if (util_timer_head == NULL)
		{
				lpc21XX_timer0_stop ();
		}
		else if (util_timer_head != qh)
		{
				lpc21XX_timer0_start (util_timer_head->x_rsecs);
		}
}

util_timer_handle_t util_timer_create (const util_timer_callback_t callback, void * const calltoken, unsigned long secs)
{
		util_timer_handle_t handle;

		assert (callback != NULL);
		assert (secs > 0);

		if (!util_timer_init)
		{
				util_timer_init = TRUE;

				lpc21XX_timer0_init (util_timer_expiry);

				for (handle = (util_timer_handle_t) 0; handle < (util_timer_handle_t) UTIL_TIMER_SIZE; ++handle)
				{
						util_timer_list [handle].secs = 0;
				}
		}

		for (handle = (util_timer_handle_t) 0; handle < (util_timer_handle_t) UTIL_TIMER_SIZE; ++handle)
		{
				if (util_timer_list [handle].secs == 0)
				{
						util_timer_t* ctx = &util_timer_list [handle];
						ctx->callback = callback;
						ctx->calltoken = calltoken;
						ctx->secs = secs;
						lpc21XX_timer0_suspend ();
						util_timer_enqueue (ctx);
						lpc21XX_timer0_resume ();
						return handle;
				}
		}

		return UTIL_TIMER_HANDLE_INVALID;
}

void util_timer_restart (util_timer_handle_t handle)
{
		util_timer_t * ctx = &util_timer_list [handle];

		assert (UTIL_TIMER_HANDLE_VALID (handle));
		assert (handle < (util_timer_handle_t) UTIL_TIMER_SIZE);

		lpc21XX_timer0_suspend ();
		util_timer_dequeue (ctx);
		util_timer_enqueue (ctx);
		lpc21XX_timer0_resume ();
}

void util_timer_destroy (util_timer_handle_t handle)
{
		util_timer_t * ctx = &util_timer_list [handle];

		assert (UTIL_TIMER_HANDLE_VALID (handle));
		assert (handle < (util_timer_handle_t) UTIL_TIMER_SIZE);

		lpc21XX_timer0_suspend ();
		util_timer_dequeue (ctx);
		lpc21XX_timer0_resume ();
		ctx->secs = 0;
}

/* ----------------------------------------------------------------------------------------------------*/

#define UTIL_SCHEDULER_SIZE 5

typedef struct
{
		/*@shared@*/ util_scheduler_callback_t callback;
		/*@null@*/ /*@shared@*/ void * calltoken;
		/*@null@*/ /*@shared@*/ volatile boolean * trigger;
}
util_scheduler_item_t;

static util_scheduler_handle_t util_scheduler_size = (util_scheduler_handle_t) 0;
static util_scheduler_item_t util_scheduler_list [UTIL_SCHEDULER_SIZE];

util_scheduler_handle_t util_scheduler_create (const util_scheduler_callback_t callback, void * const calltoken, volatile boolean * const trigger)
{
		util_scheduler_handle_t handle = util_scheduler_size;

		assert (callback != NULL);

		if (handle == (util_scheduler_handle_t) UTIL_SCHEDULER_SIZE)
		{
				return UTIL_SCHEDULER_HANDLE_INVALID;
		}

		util_scheduler_list [handle].trigger = trigger;
		util_scheduler_list [handle].callback = callback;
		util_scheduler_list [handle].calltoken = calltoken;

		util_scheduler_size += (util_scheduler_handle_t) 1;

		return handle;
}

void util_scheduler_destroy (util_scheduler_handle_t handle)
{
		assert (UTIL_SCHEDULER_HANDLE_VALID (handle));
		assert (handle < (util_scheduler_handle_t) UTIL_SCHEDULER_SIZE);

		for (; handle < util_scheduler_size - (util_scheduler_handle_t) 1; ++handle)
		{
				(void) util_memcpy ((void *)&util_scheduler_list [handle],
						(void *)&util_scheduler_list [handle + (util_scheduler_handle_t) 1],
						(unsigned int) sizeof (util_scheduler_list [handle]));
		}

		util_scheduler_size -= (util_scheduler_handle_t) 1;
}

boolean util_scheduler_process (void)
{
		util_scheduler_handle_t handle;
		unsigned int processed = 0;

		for (handle = (util_scheduler_handle_t) 0; handle < util_scheduler_size; ++handle)
		{
				if (util_scheduler_list [handle].trigger == NULL)
				{
						if ((*util_scheduler_list [handle].callback) (util_scheduler_list [handle].calltoken) == FALSE)
						{
								return FALSE;
						}
						processed++;
				}
				else if (*(util_scheduler_list [handle].trigger))
				{
						*(util_scheduler_list [handle].trigger) = FALSE;
						if ((*util_scheduler_list [handle].callback) (util_scheduler_list [handle].calltoken) == FALSE)
						{
								return FALSE;
						}
						processed++;
				}
		}

		if (processed == 0)
		{
				lpc21XX_idle ();
		}

		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

static long util_rand_value = 0;

void util_rand_seed (const unsigned int seed)
{
		util_rand_value = (long) seed;
}

unsigned int util_rand (void)
{
		long x = util_rand_value;
		long hi = x / 127773, lo = x % 127773;
		long t = (16807 * lo) - (2836 * hi);
		if (t <= 0)
		{
				t += (long) 0x7fffffff;
		}
		util_rand_value = t;
		return (unsigned int) t;
}

/* ----------------------------------------------------------------------------------------------------*/

/*@-fixedformalarray@*/
void util_clock_get (util_clock_buf_t buffer)
{
		lpc21XX_rtc_t c;

		assert (buffer != NULL);

		lpc21XX_rtc_get (&c);

		(void) util_memcpy ((void *) buffer, (const void *) c.e, LPC21XX_RTC_ELEM_XX);
}
/*@=fixedformalarray@*/


/* ----------------------------------------------------------------------------------------------------*/

/*@-fixedformalarray@*/
boolean util_clock_set (const util_clock_buf_t buffer)
{
		lpc21XX_rtc_t c;
		/*@observer@*/ static const int days_per_month [12] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

		assert (buffer != NULL);

		if (buffer [0] > (unsigned char) 119) return FALSE;
		if (buffer [1] < (unsigned char) 1 || buffer [1] > (unsigned char) 12) return FALSE;
		if (buffer [2] < (unsigned char) 1 || buffer [2] > (unsigned char) days_per_month [ buffer [1] - (unsigned char) 1 ]) return FALSE;
		if (buffer [3] > (unsigned char) 23) return FALSE;
		if (buffer [4] > (unsigned char) 59) return FALSE;
		if (buffer [5] > (unsigned char) 59) return FALSE;

		/*@-mayaliasunique@*/ (void) util_memcpy ((void *) c.e, (const void *) buffer, LPC21XX_RTC_ELEM_XX); /*@=mayaliasunique@*/

		/*@-compdef@*/ lpc21XX_rtc_set (&c); /*@=compdef@*/

		return TRUE;
}
/*@=fixedformalarray@*/

/* ----------------------------------------------------------------------------------------------------*/

unsigned int util_date_mjd (const unsigned int year, const unsigned char month, const unsigned char day)
{
		unsigned long mjd;

		assert (year >= (unsigned int) 1980);
		assert (month >= (unsigned char) 1 && month <= (unsigned char) 12);
		assert (day >= (unsigned char) 1 && day <= (unsigned char) 31);

		mjd = (((unsigned long) ((month > (unsigned char) 2) ? (year) : (year - 1)) + (unsigned long) 4800) 
				* (unsigned long) 1461 /* days per 4 years */ ) / (unsigned long) 4;
		mjd += ((unsigned long) ((month > (unsigned char) 2) ? (month - (unsigned char) 3) : (month + (unsigned char) 9)) 
				* (unsigned long) 153 /* days per 5 months */ + (unsigned long) 2) / (unsigned long) 5;
		mjd += (unsigned long) day;

		return (unsigned int) (mjd - (unsigned long) 32083 /* offset */ - (unsigned long) 2444253 /* baseline */);
}

const char * util_date_str_month (const unsigned char month)
{
		/*@observer@*/ static const char * month_to_str [12] =
				{ "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

		assert (month >= (unsigned char) 1 && month <= (unsigned char) 12);

		return month_to_str [month - (unsigned char) 1];
}

const char * util_date_str_dayofweek (const unsigned int year, const unsigned char month, const unsigned char day)
{
		/*@observer@*/ static const char * const day_to_str [7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
		/*@observer@*/ static const int day_offset [12] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
		unsigned int yy = (month < (unsigned char) 3) ? year - 1 : year;

		assert (year >= (unsigned int) 1980);
		assert (month >= (unsigned char) 1 && month <= (unsigned char) 12);
		assert (day >= (unsigned char) 1 && day <= (unsigned char) 31);

		return day_to_str [(yy + yy/4 - yy/100 + yy/400 + day_offset [month - (unsigned char) 1] + (unsigned int) day) % 7];
}

/* ----------------------------------------------------------------------------------------------------*/

#ifdef BUILTIN_PRINTF

void util_printf (const char * const format, ...)
{
		va_list ap;
		va_start (ap, format);
		(void) vprintf (format, ap);
		/*va_end (ap);*/
}

#endif

/* ----------------------------------------------------------------------------------------------------*/

#ifdef TEST_ENABLED

static void util_test_handler_timer (void * const calltoken)
{
		( *( (unsigned long *) calltoken ) ) ++;
}

static boolean util_test_handler_scheduler (void * const calltoken)
{
		( *( (unsigned long *) calltoken ) ) ++;
		return TRUE;
}

test_result_t util_test (void)
{
		char * bptr;
		unsigned int blen;
		util_timer_handle_t timerh1, timerh2;
		unsigned long counter1, counter2;
		unsigned long schedulerc;
		util_scheduler_handle_t schedulerh;
		volatile boolean schedulert;

		/* util_buffer */
		test_assert (util_buffer_get () != NULL);
		test_assert (util_buffer_len () > 0);
		bptr = (char *) util_buffer_get ();
		blen = util_buffer_len ();
		bptr [0] = (char) 0x55; bptr [1] = (char) 0xAA;
		test_assert (bptr [0] == (char) 0x55 && bptr [1] == (char) 0xAA);

		/* util_stratol */
		(void) util_stratol (bptr, (signed long) 0); test_assert (util_strcmpi (bptr, "0"));
		(void) util_stratol (bptr, (signed long) 1); test_assert (util_strcmpi (bptr, "1"));
		(void) util_stratol (bptr, (signed long) -1); test_assert (util_strcmpi (bptr, "-1"));
		(void) util_stratol (bptr, (signed long) 999999); test_assert (util_strcmpi (bptr, "999999"));
		(void) util_stratol (bptr, (signed long) (1 << 30)); test_assert (util_strcmpi (bptr, "1073741824"));
		(void) util_stratol (bptr, (signed long) 0x7fffffffL); test_assert (util_strcmpi (bptr, "2147483647"));
		(void) util_stratol (bptr, (signed long) -2147483647); test_assert (util_strcmpi (bptr, "-2147483647"));

		/* util_strcat */
		bptr [0] = '\0';
		(void) util_strcat (bptr, "foo");
		test_assert (util_strcmpi (bptr, "foo"));
		(void) util_strcat (bptr, "bar");
		test_assert (util_strcmpi (bptr, "foobar"));

		/* util_strpad */
		test_assert (util_strpad (bptr, "foo", '0', (unsigned int) 6) == (unsigned int) 6 && util_strcmpi (bptr, "000foo"));
		test_assert (util_strpad (bptr, "foo", '0', (unsigned int) 2) == (unsigned int) 3 && util_strcmpi (bptr, "foo"));
		test_assert (util_strpad (bptr, "foo", '0', (unsigned int) 3) == (unsigned int) 3 && util_strcmpi (bptr, "foo"));

		/* util_strcpy */
		(void) util_strcpy (bptr, "foo");
		test_assert (util_strcmpi (bptr, "foo"));
		(void) util_strcpy (bptr, "barfoo");
		test_assert (util_strcmpi (bptr, "barfoo"));

		/* util_strend */
		(void) util_strcpy (bptr, "foo");
		test_assert (util_strend (bptr) != 0);
		test_assert (util_strend (bptr) == &bptr [3]);

		/* util_strlen */
		bptr [0] = '\0';
		test_assert (util_strlen (bptr) == (unsigned int) 0);
		(void) util_strcpy (bptr, "foo");
		test_assert (util_strlen (bptr) == (unsigned int) 3);
		(void) util_strcat (bptr, "bar");
		test_assert (util_strlen (bptr) == (unsigned int) 6);

		/* util_strtol */
		test_assert (util_strtol ("0") == (signed long) 0);
		test_assert (util_strtol ("1") == (signed long) 1);
		test_assert (util_strtol ("-1") == (signed long) -1);
		test_assert (util_strtol ("+1") == (signed long) 1);
		test_assert (util_strtol ("999999") == (signed long) 999999);
		test_assert (util_strtol ("1073741824") == (signed long) (1 << 30));
		test_assert (util_strtol ("2147483647") == (signed long) 0x7fffffffL);
		test_assert (util_strtol ("-2147483647") == (signed long) -2147483647);
		test_assert (util_strtol ("ABC") == (signed long) 0);

		/* util_strcmpi */
		test_assert (util_strcmpi ("foo", "FOO") == TRUE);
		test_assert (util_strcmpi ("foo", "foo") == TRUE);
		test_assert (util_strcmpi ("foo", "bar") == FALSE);

		/* util_strcmpix */
		test_assert (util_strcmpix ("foo", "foo|bar", '|') == TRUE);
		test_assert (util_strcmpix ("bar", "foo|bar", '|') == TRUE);
		test_assert (util_strcmpix ("fo", "foo|bar", '|') == FALSE);
		test_assert (util_strcmpix ("foob", "foo|bar", '|') == FALSE);
		test_assert (util_strcmpix ("bar", "foo,bar", ',') == TRUE);
		test_assert (util_strcmpix ("foob", "foo,bar", ',') == FALSE);

		/* util_memcpy */
		bptr [0] = (char) 0x55; bptr [1] = (char) 0xAA; bptr [2] = (char) 0xAA; bptr [3] = (char) 0x55;
		test_assert (util_memcpy (&bptr [2], &bptr [0], (unsigned int) 2) != 0);
		test_assert (bptr [0] == (char) 0x55 && bptr [1] == (char) 0xAA && bptr [2] == (char) 0x55 && bptr [3] == (char) 0xAA);

		/* util_memcmp */
		bptr [0] = (char) 0x55; bptr [1] = (char) 0xAA; bptr [2] = (char) 0x55; bptr [3] = (char) 0xAA;
		test_assert (util_memcmp (&bptr [2], &bptr [0], (unsigned int) 2) == 0);
		test_assert (util_memcmp (&bptr [1], &bptr [0], (unsigned int) 2) != 0);

		/* util_memset */
		test_assert (util_memset (&bptr [0], (unsigned char) 0x55, (unsigned int) 8) != 0);
		test_assert (util_memset (&bptr [0], (unsigned char) 0xAA, (unsigned int) 6) != 0);
		test_assert (bptr [0] == (char) 0xAA && bptr [1] == (char) 0xAA && bptr [5] == (char) 0xAA && bptr [6] == (char) 0x55);

		/* util_packl */
		test_assert (util_binpack ((unsigned char *)bptr, (unsigned int) 0, (unsigned long) 0x12345678L, (unsigned int) 8) >> 3 == (unsigned int) 1);
		test_assert (bptr [0] == (char) 0x78);
		test_assert (util_binpack ((unsigned char *)bptr, (unsigned int) 0, (unsigned long) 0x12345678L, (unsigned int) 24) >> 3 == (unsigned int) 3);
		test_assert (bptr [0] == (char) 0x34 && bptr [1] == (char) 0x56 && bptr [2] == (char) 0x78);
		bptr [0] = (char) 0x5F;
		test_assert (util_binpack ((unsigned char *)bptr, (unsigned int) 4, (unsigned long) 0x12345678L, (unsigned int) 4) == (unsigned int) 8);
		test_assert (bptr [0] == (char) 0x58);
		bptr [0] = (char) 0x8F;
		test_assert (util_binpack ((unsigned char *)bptr, (unsigned int) 3, (unsigned long) 0x12345678L, (unsigned int) 32) == (unsigned int) 35);
		test_assert (bptr [0] == (char) 0x82 && bptr [1] == (char) 0x46 && bptr [2] == (char) 0x8A && bptr [3] == (char) 0xCF && bptr [4] == (char) 0x00);
		bptr [0] = (char) 0x00;
		test_assert (util_binpack ((unsigned char *)bptr, (unsigned int) 0, (unsigned long) 0x00000001L, (unsigned int) 1) == (unsigned int) 1);
		test_assert (util_binpack ((unsigned char *)bptr, (unsigned int) 1, (unsigned long) 0x00000000L, (unsigned int) 1) == (unsigned int) 2);
		test_assert (util_binpack ((unsigned char *)bptr, (unsigned int) 2, (unsigned long) 0x00000001L, (unsigned int) 1) == (unsigned int) 3);
		test_assert (util_binpack ((unsigned char *)bptr, (unsigned int) 3, (unsigned long) 0x00000001L, (unsigned int) 1) == (unsigned int) 4);
		test_assert (bptr [0] == (char) 0xB0);

		/* util_makenumberedfn */
		(void) util_makenumberedfn (bptr, "ZZZ", "YYY", (unsigned int) 0);
		test_assert (util_strcmpi (bptr, "ZZZ00000.YYY"));
		(void) util_makenumberedfn (bptr, "ZZZ", "YYY", (unsigned int) 999);
		test_assert (util_strcmpi (bptr, "ZZZ00999.YYY"));
		(void) util_makenumberedfn (bptr, "ZZZZZ", "YYY", (unsigned int) 999);
		test_assert (util_strcmpi (bptr, "ZZZZZ999.YYY"));

		/* util_csv_field_extract */
		test_assert (util_csv_field_extract ("foo,bar", bptr, (unsigned char) (3+1)) != NULL);
		test_assert (util_strcmpi (bptr, "foo"));
		test_assert (util_csv_field_extract ("foowoo,bar", bptr, (unsigned char) (4+1)) != NULL);
		test_assert (util_strcmpi (bptr, "foow"));
		test_assert (util_csv_field_extract (",,,", bptr, (unsigned char) (3+1)) != NULL);
		test_assert (bptr [0] == '\0');

		/* util_csv_field_next */
		test_assert (util_csv_field_next ("foo,bar") != NULL);
		test_assert (util_csv_field_next (",,,") != NULL);

		/* util_timer */
		counter1 = counter2 = (unsigned long) 0;
		timerh1 = util_timer_create (util_test_handler_timer, (void *) &counter1, (unsigned long) 2);
		timerh2 = util_timer_create (util_test_handler_timer, (void *) &counter2, (unsigned long) 3);
		/*@i@*/ while (*((volatile unsigned long *) &counter2) != (unsigned long) 1)
			/*@i@*/ lpc21XX_idle ();
		test_assert (*((volatile unsigned long *) &counter1) == (unsigned long) 1);
		util_timer_restart (timerh1);
		util_timer_restart (timerh2);
		/*@i@*/ while (*((volatile unsigned long *) &counter1) != (unsigned long) 2)
			/*@i@*/ lpc21XX_idle ();
		util_timer_destroy (timerh1);
		util_timer_destroy (timerh2);

		/* util_scheduler */
		util_scheduler_size = (util_scheduler_handle_t) 0;
		schedulerc = (unsigned long) 0;
		schedulert = FALSE;
		schedulerh = util_scheduler_create (util_test_handler_scheduler, (void *) &schedulerc, &schedulert);
		(void) util_scheduler_process ();
		test_assert (!schedulert && schedulerc == (unsigned long) 0);
		schedulert = TRUE;
		(void) util_scheduler_process ();
		test_assert (!schedulert && schedulerc == (unsigned long) 1);
		util_scheduler_destroy (schedulerh);
	   
		/* util_rand */ 
/*@-evalorderuncon@*/
		util_rand_seed ((unsigned int) 112312);
		test_assert (util_rand () != util_rand ());
		test_assert (util_rand () != util_rand ());
		test_assert (util_rand () != util_rand ());
/*@=evalorderuncon@*/

		/* util_clock */

		/* util_date_mjd */
		test_assert (util_date_mjd ((unsigned int) 1980, (unsigned char) 1, (unsigned char) 1) == (unsigned int) 0);
		test_assert (util_date_mjd ((unsigned int) 2007, (unsigned char) 1, (unsigned char) 1) == (unsigned int) 9862);
		test_assert (util_date_mjd ((unsigned int) 2024, (unsigned char) 1, (unsigned char) 1) == (unsigned int) 16071);

		/* util_printf */

		/* UTIL_TOUPPER */
		test_assert (UTIL_TOUPPER ('a') == 'A');
		test_assert (UTIL_TOUPPER ('A') == 'A');
		test_assert (UTIL_TOUPPER ('$') == '$');
		test_assert (UTIL_TOUPPER ('1') == '1');

		/* UTIL_MIN */
		test_assert (UTIL_MIN (10, 20) == 10);
		test_assert (UTIL_MIN (-1, 5) == -1);
		test_assert (UTIL_MIN (5, 5) == 5);

		/* UTIL_ISSPACE */
		test_assert (UTIL_ISSPACE (' '));
		test_assert (UTIL_ISSPACE ('\r'));
		test_assert (!UTIL_ISSPACE ('_'));

		/* UTIL_ISDIGIT */
		test_assert (UTIL_ISDIGIT ('5'));
		test_assert (!UTIL_ISDIGIT ('a'));

		return TEST_RESULT_OKAY;
}

#endif

/* ----------------------------------------------------------------------------------------------------*/
