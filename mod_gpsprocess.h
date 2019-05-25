
/* ----------------------------------------------------------------------------------------------------*/
/*
	mod_gpsprocess.h: gps logger module for GPS processing.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#ifndef MOD_GPSPROCESS_H_
#define MOD_GPSPROCESS_H_

/* ----------------------------------------------------------------------------------------------------*/

/*XXX*/

typedef struct {
		signed char sign;
		unsigned char major;
		unsigned long minor;
} gps_nmea_pos_t;

typedef struct {
		char utc [7]; /* HHMMSS (fractional NMEA .SS ignored) */
		gps_nmea_pos_t lat;
		gps_nmea_pos_t lon;
		signed short alt;
} gps_nmea_sentence_gga_t;

typedef struct {
		char utc [7]; /* HHMMSS (fractional NMEA .SS ignored) */
		char dd [2];
		char mm [2];
		char yyyy [4];
} gps_nmea_sentence_zda_t;

typedef struct {
		boolean valid;
		char utc [7]; /* HHMMSS (fractional NMEA .SS ignored) */
		char dd [2];
		char mm [2];
		char yy [2];
		gps_nmea_pos_t lat;
		gps_nmea_pos_t lon;
} gps_nmea_sentence_rmc_t;

typedef enum {
		gps_nmea_sentence_type_none = 0x00,
		gps_nmea_sentence_type_gga = 0x01,
		gps_nmea_sentence_type_gll = 0x02,
		gps_nmea_sentence_type_gsa = 0x04,
		gps_nmea_sentence_type_gsv = 0x08,
		gps_nmea_sentence_type_rmc = 0x10,
		gps_nmea_sentence_type_vtg = 0x20,
		gps_nmea_sentence_type_zda = 0x40,
		gps_nmea_sentence_type_oth = 0x40,
		gps_nmea_sentence_type_all = 0xFF
} gps_nmea_sentence_type_t;

typedef struct {
		gps_nmea_sentence_type_t type;
		union {
				gps_nmea_sentence_gga_t gga;
				gps_nmea_sentence_rmc_t rmc;
				gps_nmea_sentence_zda_t zda;
		} stru;
} gps_nmea_sentence_t;

/* ----------------------------------------------------------------------------------------------------*/

typedef struct {
		boolean update_rtc;
} gps_config_process_t;

/* ----------------------------------------------------------------------------------------------------*/

boolean gps_process_init (/*@notnull@*/ const gps_config_process_t * const config);
boolean gps_process_term (void);

boolean gps_process_handle (const gps_nmea_sentence_t * const content);

#ifdef TEST_ENABLED
test_result_t gps_process_test (void);
#endif

/* ----------------------------------------------------------------------------------------------------*/

#endif /*MOD_GPSPROCESS_H_*/

