
/* ----------------------------------------------------------------------------------------------------*/
/*
	common.h: gps logger common definitions, variables, functions, etc.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#ifndef COMMON_H_
#define COMMON_H_

/* ----------------------------------------------------------------------------------------------------*/

#define VERSION "0.94b-dev"
#define COPYRIGHT "copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved."
#ifdef HARDWARE_24 
#define HARDWARE "sparkfun-2.4/lpc21xx/em406"
#endif
#ifdef HARDWARE_10
#define HARDWARE "sparkfun-1.0/lpc21xx/iQ"
#endif

/* ----------------------------------------------------------------------------------------------------*/

#ifndef NULL
#define NULL ((void * )0)
#endif

/* ----------------------------------------------------------------------------------------------------*/

typedef unsigned char boolean;

#define TRUE ((boolean) 1)
#define FALSE ((boolean) 0)

/* ----------------------------------------------------------------------------------------------------*/

/*@-namechecks@*/ extern int __putchar (int); /*@=namechecks@*/
#define putchar __putchar

/* ----------------------------------------------------------------------------------------------------*/

#ifdef DEBUG
/*@noreturnwhenfalse@*/ /*@-namechecks@*/ void __assert (/*@notnull@*/ const char* const filename, /*@notnull@*/ const unsigned int fileline, /*@notnull@*/ const char* const expression) __attribute__ ((noreturn)); /*@=namechecks@*/
#define assert(x) do { /*@-namechecks@*/ if (!(x)) __assert (__FILE__, (unsigned int) __LINE__, #x); /*@=namechecks@*/ } while (FALSE)
#else
/*@i@*/ #define assert(x)
#endif

#ifdef DEBUG
extern boolean debug_trace;
#define DPRINTF(x) if (debug_trace) util_printf x
#ifndef BUILTIN_PRINTF
#define BUILTIN_PRINTF
#endif
#else
/*@i@*/ #define DPRINTF(x)
#endif

#define DEBUG_MODE_LIST "none|trace|diag"
#define DEBUG_MODE_DEFAULT	"trace"

/* ----------------------------------------------------------------------------------------------------*/

#define HALT(x) do { DPRINTF (x); lpc21XX_halt (); } while (FALSE)

/* ----------------------------------------------------------------------------------------------------*/

#ifdef DEBUG

#define TEST_ENABLED

#define TEST_RESULT_OKAY  ((test_result_t) 0)

typedef int test_result_t;

#define test_assert(x) do { if (!(x)) { return (test_result_t) __LINE__; } } while (FALSE)
#define test_runner(f) do { test_result_t l; DPRINTF (("*** test_execute: %s ***\n", #f)); if ((l = f ()) != TEST_RESULT_OKAY) { HALT (("test failed: test %s at line %d\n", #f, l)); } } while (FALSE)
#define test_trying(m) do { DPRINTF (("--- test_trying: %s ---\n", m)); } while (FALSE)

#endif

/* ----------------------------------------------------------------------------------------------------*/

#ifdef BUILTIN_PRINTF
void util_printf (/*@notnull@*/ const char * const format, ...) __attribute__ ((format (printf, 1, 2)));
#endif

/* ----------------------------------------------------------------------------------------------------*/

#define PACKLZARI_ENCODE
#if defined (DEBUG) || defined (TEST_ENABLED)
#define PACKLZARI_DECODE
#endif

/* ----------------------------------------------------------------------------------------------------*/

#ifdef INLINE
#define INLINE_DECL extern inline __attribute__ ((always_inline))
#else
#define INLINE_DECL
#endif

#define UNUSED(arg) /*@-noeffect@*/ (void) arg /*@=noeffect@*/

/* ----------------------------------------------------------------------------------------------------*/

#endif /*COMMON_H_*/

