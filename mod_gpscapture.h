
/* ----------------------------------------------------------------------------------------------------*/
/*
	mod_gpscapture.h: gps logger module for gps position capturing.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#ifndef MOD_GPSCAPTURE_H_
#define MOD_GPSCAPTURE_H_

/* ----------------------------------------------------------------------------------------------------*/

boolean gps_capture_suspend (void);
boolean gps_capture_resume (void);

/*@observer@*/ /*@notnull@*/ const char * gps_capture_filename (void);

boolean gps_capture_clear (void);

/*@observer@*/ /*@notnull@*/ const char * gps_capture_get (void);

boolean gps_capture_save (/*@null@*/ const char * const description);

/* ----------------------------------------------------------------------------------------------------*/

#ifdef TEST_ENABLED
test_result_t gps_capture_test (void);
#endif

/* ----------------------------------------------------------------------------------------------------*/

#endif /*MOD_GPSCAPTURE_H_*/

