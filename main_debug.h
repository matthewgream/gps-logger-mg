
/* ----------------------------------------------------------------------------------------------------*/
/*
	main_debug.h: gps logger main debug mechanisms.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#ifndef MAIN_DEBUG_H_
#define MAIN_DEBUG_H_

/* ----------------------------------------------------------------------------------------------------*/

#ifdef DEBUG

void debug_init (void);
void debug_term (void);
boolean debug_mode (/*@notnull@*/ const char * const mode);

#endif

/* ----------------------------------------------------------------------------------------------------*/

#endif /*MAIN_DEBUG_H_*/

