
/* ----------------------------------------------------------------------------------------------------*/
/*
	mod_gpsoutput.h: gps logger module for GPS processing.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#ifndef MOD_GPSOUTPUT_H_
#define MOD_GPSOUTPUT_H_

/* ----------------------------------------------------------------------------------------------------*/

typedef struct
{
		boolean enabled;
		unsigned int buffer_size;
		unsigned long timeout_normal;
		unsigned long timeout_quiet;
}
gps_config_mode_file_t;

typedef struct
{
		boolean enabled;
		unsigned int serial_speed;
}
gps_config_mode_pass_t;

typedef struct
{
		gps_config_mode_file_t file;
		gps_config_mode_pass_t pass;
		boolean compress;
}
gps_config_output_t;

/* ----------------------------------------------------------------------------------------------------*/

#define GPS_MODE_LIST "file|pass"
#define GPS_MODE_DEFAULT "file"

/* ----------------------------------------------------------------------------------------------------*/

boolean gps_output_init (/*@notnull@*/ const gps_config_output_t* const config);
boolean gps_output_term (void);

boolean gps_output_write (/*@notnull@*/ const unsigned char* const buffer, const unsigned int length);

#ifdef TEST_ENABLED
test_result_t gps_output_test (void);
#endif

/* ----------------------------------------------------------------------------------------------------*/

#endif /*MOD_GPSOUTPUT_H_*/

