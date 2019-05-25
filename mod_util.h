
/* ----------------------------------------------------------------------------------------------------*/
/*
	mod_util.h: gps logger module for utility functions.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#ifndef MOD_UTIL_H_
#define MOD_UTIL_H_

/* ----------------------------------------------------------------------------------------------------*/

#define UTIL_BUFFER_SIZE 24576

/* ----------------------------------------------------------------------------------------------------*/

#define UTIL_MIN(a,b) (((a) < (b)) ? (a) : (b))
#define UTIL_TOUPPER(c) (((c) >= 'a' && (c) <= 'z') ? (((c) - 'a') + 'A') : (c))
#define UTIL_ISSPACE(c) ((c) == ' ' || (c) == '\t' || (c) == '\r' || (c) == '\n')
#define UTIL_ISDIGIT(c) ((c) >= '0' && (c) <= '9')
#define UTIL_ABS(a) (((a) < 0) ? -(a) : (a))

#define util_str(x) #x
#define util_xstr(x) util_str (x)

/* ----------------------------------------------------------------------------------------------------*/

boolean util_strcmpi (/*@notnull@*/ const char * const namea, /*@notnull@*/ const char * const nameb);
boolean util_strcmpix (/*@notnull@*/ const char * const name, /*@notnull@*/ const char * const list, const char sep);
unsigned int util_stratol (/*@notnull@*/ /*@out@*/ char * const string, const signed long integer);
unsigned int util_strpad (/*@notnull@*/ /*@out@*/ char * const string, /*@notnull@*/ const char * const content, const char pad, const unsigned int len);
unsigned int util_strcat (/*@notnull@*/ char * const string, /*@notnull@*/ const char * const content);
unsigned int util_strcpy (/*@notnull@*/ /*@out@*/ char * const string, /*@notnull@*/ const char * const content);
unsigned int util_strlen (/*@notnull@*/ const char * const content);
signed long util_strtol (/*@notnull@*/ const char * const content);
/*@notnull@*/ char * util_strend (/*@returned@*/ /*@notnull@*/ char * const string);

/* ----------------------------------------------------------------------------------------------------*/

/*@notnull@*/ char * util_makenumberedfn (/*@returned@*/ /*@notnull@*/ char * const string, /*@notnull@*/ const char * const prefix, /*@notnull@*/ const char * const suffix, const unsigned int count);

/* ----------------------------------------------------------------------------------------------------*/

/*@notnull@*/ void * util_memcpy (/*@returned@*/ /*@unique@*/ /*@out@*/ /*@notnull@*/ void * const dst, /*@notnull@*/ const void * const src, const unsigned int len);
/*@-exportlocal@*/
int util_memcmp (/*@notnull@*/ const void * const dst, /*@notnull@*/ const void * const src, const unsigned int len);
/*@=exportlocal@*/
/*@notnull@*/ void * util_memset (/*@returned@*/ /*@unique@*/ /*@out@*/ /*@notnull@*/ void * const dst, /*@notnull@*/ const unsigned char chr, const unsigned int len);

/* ----------------------------------------------------------------------------------------------------*/

unsigned int util_binpack (/*@notnull@*/ unsigned char * const data, const unsigned int offs, const unsigned long integer, const unsigned int nbits);

/* ----------------------------------------------------------------------------------------------------*/

/*@shared@*/ /*@null@*/ unsigned char * util_buffer_alloc (const unsigned int size);
void util_buffer_reset (void);
/*@shared@*/ /*@notnull@*/ unsigned char * util_buffer_get (void);
unsigned int util_buffer_len (void);

/* ----------------------------------------------------------------------------------------------------*/

/*@null@*/ const char * util_csv_field_extract (/*@returned@*/ /*@notnull@*/ const char * const input, /*@out@*/ /*@notnull@*/ char * const output, const unsigned char size);
/*@null@*/ const char * util_csv_field_next (/*@returned@*/ /*@notnull@*/ const char * const input);

/* ----------------------------------------------------------------------------------------------------*/

typedef void (*util_timer_callback_t) (/*@null@*/ void * const calltoken);
typedef unsigned char util_timer_handle_t;
#define UTIL_TIMER_HANDLE_INVALID  ((util_timer_handle_t) -1)
#define UTIL_TIMER_HANDLE_VALID(v) ((v) != UTIL_TIMER_HANDLE_INVALID)

/*@null@*/ util_timer_handle_t util_timer_create (/*@notnull@*/ /*@shared@*/ const util_timer_callback_t callback, /*@null@*/ /*@shared@*/ void * const calltoken, unsigned long secs);
void util_timer_restart (util_timer_handle_t handle);
void util_timer_destroy (util_timer_handle_t handle);

/* ----------------------------------------------------------------------------------------------------*/

typedef boolean (*util_scheduler_callback_t) (/*@null@*/ void * const calltoken);
typedef unsigned char util_scheduler_handle_t;
#define UTIL_SCHEDULER_HANDLE_INVALID	((util_scheduler_handle_t) -1)
#define UTIL_SCHEDULER_HANDLE_VALID(v) ((v) != UTIL_SCHEDULER_HANDLE_INVALID)

/*@null@*/ util_scheduler_handle_t util_scheduler_create (/*@notnull@*/ /*@shared@*/ const util_scheduler_callback_t callback, /*@null@*/ /*@shared@*/ void * const calltoken, /*@null@*/ /*@shared@*/ volatile boolean * const trigger);
void util_scheduler_destroy (util_scheduler_handle_t handle);
boolean util_scheduler_process (void);

/* ----------------------------------------------------------------------------------------------------*/

void util_rand_seed (const unsigned int seed);
unsigned int util_rand (void);

/* ----------------------------------------------------------------------------------------------------*/

/*@-fixedformalarray@*/
typedef unsigned char util_clock_buf_t [7];
void util_clock_get (/*@notnull@*/ /*@out@*/ util_clock_buf_t buffer);
boolean util_clock_set (/*@notnull@*/ const util_clock_buf_t buffer);
/*@=fixedformalarray@*/

/* ----------------------------------------------------------------------------------------------------*/

unsigned int util_date_mjd (const unsigned int year, const unsigned char month, const unsigned char day);
/*@observer@*/ const char * util_date_str_month (const unsigned char month);
/*@observer@*/ const char * util_date_str_dayofweek (const unsigned int year, const unsigned char month, const unsigned char day);

/* ----------------------------------------------------------------------------------------------------*/

#ifdef TEST_ENABLED
test_result_t util_test (void);
#endif

/* ----------------------------------------------------------------------------------------------------*/

#ifdef INLINE
#include "mod_util.i"
#endif

/* ----------------------------------------------------------------------------------------------------*/

#endif /*MOD_UTIL_H_*/

