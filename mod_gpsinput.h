
/* ----------------------------------------------------------------------------------------------------*/
/*
	mod_gpsinput.h: gps logger module for GPS processing.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#ifndef MOD_GPSINPUT_H_
#define MOD_GPSINPUT_H_

/* ----------------------------------------------------------------------------------------------------*/

typedef struct {
		unsigned long interval;
} gps_config_input_t;

/* ----------------------------------------------------------------------------------------------------*/

boolean gps_input_init (/*@notnull@*/ const gps_config_input_t * const config);
boolean gps_input_term (void);

unsigned int gps_input_read (/*@out@*/ unsigned char * data, const unsigned int size);

#ifdef TEST_ENABLED
test_result_t gps_input_test (void);
#endif

/* ----------------------------------------------------------------------------------------------------*/

#endif /*MOD_GPSINPUT_H_*/

