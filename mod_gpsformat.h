
/* ----------------------------------------------------------------------------------------------------*/
/*
	mod_gpsformat.h: gps logger module for GPS processing.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#ifndef MOD_GPSFORMAT_H_
#define MOD_GPSFORMAT_H_

/* ----------------------------------------------------------------------------------------------------*/

typedef enum
{
		gps_config_format_raw,
		gps_config_format_nmea,
		gps_config_format_kml,
		gps_config_format_csv,
		gps_config_format_time
}
gps_config_format_type_t;

typedef struct
{
		gps_config_format_type_t format;
		/*@shared@*/ const char * nmea_sentences;
		/*@shared@*/ const char * csv_content;
		boolean csv_binary;
}
gps_config_format_t;

/* ----------------------------------------------------------------------------------------------------*/

#define GPS_FORMAT_LIST "raw|nmea|kml|csv|time"
#define GPS_FORMAT_DEFAULT "kml"

#define GPS_FORMAT_CSV_ELEMENT_LIST "lon|lat|alt|tim|dat"
#define GPS_FORMAT_CSV_ELEMENT_DEFAULT "lon,lat,alt,tim"

#define GPS_FORMAT_CSV_ENCODING_LIST "text|binary"
#define GPS_FORMAT_CSV_ENCODING_DEFAULT "text"

#define GPS_FORMAT_NMEA_SENTENCE_LIST "rmc|gga|gll|vtg|gsv|gsa|zda|***"
#define GPS_FORMAT_NMEA_SENTENCE_DEFAULT "gga,zda"

/* ----------------------------------------------------------------------------------------------------*/

boolean gps_format_init (/*@notnull@*/ const gps_config_format_t * const config);
unsigned int gps_format_term (/*@notnull@*/ /*@out@*/ unsigned char * str, const unsigned int size);

unsigned int gps_format_process (/*@notnull@*/ unsigned char * str, const unsigned int size);

#ifdef TEST_ENABLED
test_result_t gps_format_test (void);
#endif

/* ----------------------------------------------------------------------------------------------------*/

#endif /*MOD_GPSFORMAT_H_*/

