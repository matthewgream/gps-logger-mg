
/* ----------------------------------------------------------------------------------------------------*/
/*
	mod_gps.h: gps logger module for GPS processing.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#ifndef MOD_GPS_H_
#define MOD_GPS_H_

/* ----------------------------------------------------------------------------------------------------*/

#include "mod_gpsinput.h"
#include "mod_gpsformat.h"
#include "mod_gpsprocess.h"
#include "mod_gpsoutput.h"
#include "mod_gpscapture.h"

/* ----------------------------------------------------------------------------------------------------*/

typedef struct
{
		gps_config_input_t input;
		gps_config_format_t format;
		gps_config_process_t process;
		gps_config_output_t output;
		/*gps_config_capture_t capture;*/
}
gps_config_t;

/* ----------------------------------------------------------------------------------------------------*/

boolean gps_init (/*@notnull@*/ const gps_config_t * const config);
boolean gps_term (void);

#ifdef TEST_ENABLED
test_result_t gps_test (void);
#endif

/* ----------------------------------------------------------------------------------------------------*/

#endif /*MOD_GPS_H_*/

