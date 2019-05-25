
/* ----------------------------------------------------------------------------------------------------*/
/*
	mod_gpsformat.c: gps logger module for GPS processing.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#include "common.h"
#include "mod_util.h"
#include "mod_gpsformat.h"
#include "mod_gpsprocess.h"

/* ----------------------------------------------------------------------------------------------------*/

static unsigned char gps_fromhexchr (const char ch)
{
		return (ch >= '0' && ch <= '9') ? (unsigned char) (ch - '0') : (unsigned char) ((UTIL_TOUPPER (ch) - 'A') + (char) 10);
}

/* ----------------------------------------------------------------------------------------------------*/

#ifdef XXX

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
		gps_nmea_sentence_type_oth = 0x80,
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

#endif

static boolean gps_nmea_decode_GGA (gps_nmea_sentence_gga_t * const state, const char * const input)
{
		char utc [15], lat [15], lat_dir [2], lon [15], lon_dir [2], alt [15], tmp [4];
		signed long tmpl;
		const char * pntr;

		assert (state != NULL);
		assert (input != NULL);

		assert (input [0] == '$' && input [1] == 'G' && input [2] == 'P');
		assert (input [3] == 'G' && input [4] == 'G' && input [5] == 'A' && input [6] == ',');

		if ((pntr = util_csv_field_extract (&input [7], utc, (unsigned char) sizeof (utc))) == NULL || utc [0] == '\0') /* UTC */
				return FALSE;
		if ((pntr = util_csv_field_extract (pntr, lat, (unsigned char) sizeof (lat))) == NULL || lat [0] == '\0') /* Latitude */
				return FALSE;
		if ((pntr = util_csv_field_extract (pntr, lat_dir, (unsigned char) sizeof (lat_dir))) == NULL || lat_dir [0] == '\0') /* North/South */
				return FALSE;
		if ((pntr = util_csv_field_extract (pntr, lon, (unsigned char) sizeof (lon))) == NULL || lon [0] == '\0') /* Longitude */
				return FALSE;
		if ((pntr = util_csv_field_extract (pntr, lon_dir, (unsigned char) sizeof (lon_dir))) == NULL || lon_dir [0] == '\0') /* East/West */
				return FALSE;
		if ((pntr = util_csv_field_next (pntr)) == NULL) /* ?? n */
				return FALSE;
		if ((pntr = util_csv_field_next (pntr)) == NULL) /* Number of Satellites */
				return FALSE;
		if ((pntr = util_csv_field_next (pntr)) == NULL) /* ?? pp.p */
				return FALSE;
		if ((pntr = util_csv_field_extract (pntr, alt, (unsigned char) sizeof (alt))) == NULL) /* Altitude */
				return FALSE;

		if (!UTIL_ISDIGIT (utc [0]) || !UTIL_ISDIGIT (utc [1]) || !UTIL_ISDIGIT (utc [2]) ||
			!UTIL_ISDIGIT (utc [3]) || !UTIL_ISDIGIT (utc [4]) || !UTIL_ISDIGIT (utc [5]))
				return FALSE;
		if (lat_dir [0] != 'S' && lat_dir [0] != 'N')
				return FALSE;
		if (lon_dir [0] != 'E' && lon_dir [0] != 'W')
				return FALSE;

		(void) util_memcpy (state->utc, utc, (unsigned int) 6);
		state->utc [6] = '\0';

		/* lat is in format DDMM.mmmmmmm* -> convert to DD + (MM.mmmmmmmm / 60) */
		tmp [0] = lat [0]; tmp [1] = lat [1]; tmp [2]= '\0';
		state->lat.major = (unsigned char) util_strtol (tmp);
		state->lat.sign = (lat_dir [0] == 'S') ? (signed char) -1 : (signed char) 1;
		lat [4] = lat [3]; lat [3] = lat [2];
		state->lat.minor = (unsigned long) ((util_strtol (&lat [3]) * 5L) / 3L); /* 100L / 60L */

		/* lon is in format DDDMM.mmmmmmm* -> convert to DDD + (MM.mmmmmmmm / 60) */
		tmp [0] = lon [0]; tmp [1] = lon [1]; tmp [2] = lon [2]; tmp [3] = '\0';
		state->lon.major = (unsigned char) util_strtol (tmp);
		state->lon.sign = (lon_dir [0] == 'W') ? (signed char) -1 : (signed char) 1;
		lon [5] = lon [4]; lon [4] = lon [3];
		state->lon.minor = (unsigned long) ((util_strtol (&lon [4]) * 5L) / 3L); /* 100L / 60L */

		tmpl = util_strtol (alt);
#ifdef EMULATE
		/* the Lassen iQ and EM-406 are rated to 18,000m - anything higher than this, we clamp */
		if (tmpl > (signed long) 32767 || tmpl <= (signed long) -32767) {
				DPRINTF (("gps_nmea_decode_GGA: clamping altitude %ld to +/-32767\n", tmpl));
/*@-elseifcomplete@*/
				if (tmpl > (signed long) 32767) tmpl = (signed long) 32767;
				else if (tmpl < (signed long)-32767) tmpl = (signed long) -32767;
/*@=elseifcomplete@*/
		}
#endif
		state->alt = (signed short) tmpl;

		return TRUE;
}

static boolean gps_nmea_decode_ZDA (gps_nmea_sentence_zda_t * const state, const char * const input)
{
		char utc [15], dd [4], mm [4], yyyy [6];
		const char * pntr;

		assert (state != NULL);
		assert (input != NULL);

		assert (input [0] == '$' && input [1] == 'G' && input [2] == 'P');
		assert (input [3] == 'Z' && input [4] == 'D' && input [5] == 'A' && input [6] == ',');

		if ((pntr = util_csv_field_extract (&input [7], utc, (unsigned char) sizeof (utc))) == NULL || utc [0] == '\0') /* UTC */
				return FALSE;
		if ((pntr = util_csv_field_extract (pntr, dd, (unsigned char) sizeof (dd))) == NULL || dd [0] == '\0') /* Day */
				return FALSE;
		if ((pntr = util_csv_field_extract (pntr, mm, (unsigned char) sizeof (mm))) == NULL || mm [0] == '\0') /* Month */
				return FALSE;
		if ((pntr = util_csv_field_extract (pntr, yyyy, (unsigned char) sizeof (yyyy))) == NULL || yyyy [0] == '\0') /* Year */
				return FALSE;

		(void) util_memcpy (state->utc, utc, (unsigned int) 6);
		state->utc [6] = '\0';

		state->dd [0] = dd [0]; state->dd [1] = dd [1];

		state->mm [0] = mm [0]; state->mm [1] = mm [1];

		state->yyyy [0] = yyyy [0]; state->yyyy [1] = yyyy [1];
		state->yyyy [2] = yyyy [2]; state->yyyy [3] = yyyy [3];

		return TRUE;
}

static boolean gps_nmea_decode_RMC (gps_nmea_sentence_rmc_t * const state, const char * const input)
{
		char utc [15], valid [2], ddmmyy [7], lat [15], lat_dir [2], lon [15], lon_dir [2], tmp [4];
		const char * pntr;

		assert (state != NULL);
		assert (input != NULL);

		assert (input [0] == '$' && input [1] == 'G' && input [2] == 'P');
		assert (input [3] == 'R' && input [4] == 'M' && input [5] == 'C' && input [6] == ',');

		if ((pntr = util_csv_field_extract (&input [7], utc, (unsigned char) sizeof (utc))) == NULL || utc [0] == '\0') /* UTC */
				return FALSE;
		if ((pntr = util_csv_field_extract (pntr, valid, (unsigned char) sizeof (valid))) == NULL || valid [0] == '\0') /* Status */
				return FALSE;
		if ((pntr = util_csv_field_extract (pntr, lat, (unsigned char) sizeof (lat))) == NULL || lat [0] == '\0') /* Latitude */
				return FALSE;
		if ((pntr = util_csv_field_extract (pntr, lat_dir, (unsigned char) sizeof (lat_dir))) == NULL || lat_dir [0] == '\0') /* North/South */
				return FALSE;
		if ((pntr = util_csv_field_extract (pntr, lon, (unsigned char) sizeof (lon))) == NULL || lon [0] == '\0') /* Longitude */
				return FALSE;
		if ((pntr = util_csv_field_extract (pntr, lon_dir, (unsigned char) sizeof (lon_dir))) == NULL || lon_dir [0] == '\0') /* East/West */
				return FALSE;
		if ((pntr = util_csv_field_next (pntr)) == NULL) /* Speed Over Ground */
				return FALSE;
		if ((pntr = util_csv_field_next (pntr)) == NULL) /* Course Over Ground */
				return FALSE;
		if ((pntr = util_csv_field_extract (pntr, ddmmyy, (unsigned char) sizeof (ddmmyy))) == NULL || ddmmyy [0] == '\0') /* ddmmyy */
				return FALSE;

		state->valid = (valid [0] == 'A' ? TRUE : FALSE);

		(void) util_memcpy (state->utc, utc, (unsigned int) 6);
		state->utc [6] = '\0';

		state->dd [0] = ddmmyy [0]; state->dd [1] = ddmmyy [1];
		state->mm [0] = ddmmyy [2]; state->mm [1] = ddmmyy [3];
		state->yy [0] = ddmmyy [4]; state->yy [1] = ddmmyy [5];

		/* lat is in format DDMM.mmmmmmm* -> convert to DD + (MM.mmmmmmmm / 60) */
		tmp [0] = lat [0]; tmp [1] = lat [1]; tmp [2]= '\0';
		state->lat.major = (unsigned char) util_strtol (tmp);
		state->lat.sign = (lat_dir [0] == 'S') ? (signed char) -1 : (signed char) 1;
		lat [4] = lat [3]; lat [3] = lat [2];
		state->lat.minor = (unsigned long) ((util_strtol (&lat [3]) * 5L) / 3L); /* 100L / 60L */

		/* lon is in format DDDMM.mmmmmmm* -> convert to DDD + (MM.mmmmmmmm / 60) */
		tmp [0] = lon [0]; tmp [1] = lon [1]; tmp [2] = lon [2]; tmp [3] = '\0';
		state->lon.major = (unsigned char) util_strtol (tmp);
		state->lon.sign = (lon_dir [0] == 'W') ? (signed char) -1 : (signed char) 1;
		lon [5] = lon [4]; lon [4] = lon [3];
		state->lon.minor = (unsigned long) ((util_strtol (&lon [4]) * 5L) / 3L); /* 100L / 60L */

		return TRUE;
}

static boolean gps_nmea_decode_verify_checksum (/*@notnull@*/ const char * const str)
{
		const char * str_ptr = str;
		unsigned char csum = (unsigned char) 0;

		assert (str != NULL);

		while (*str_ptr != '\0' && *str_ptr != '$')
				str_ptr++;
		if (*str_ptr == '\0')
				return FALSE;
		str_ptr++;

		while (*str_ptr != '\0' && *str_ptr != '*')
				csum ^= (unsigned char) *str_ptr++;
		if (*str_ptr == '\0')
				return FALSE;
		str_ptr++;

/*@-evalorderuncon@*/
		return ((gps_fromhexchr (str_ptr [0]) << 4) | (gps_fromhexchr (str_ptr [1]))) == csum ? TRUE : FALSE;
/*@=evalorderuncon@*/
}

#define GPS_NMEA_MATCH(s,c1,c2,c3,r) if (s [3] == c1 && s [4] == c2 && s [5] == c3) return r

static gps_nmea_sentence_type_t gps_nmea_decode_identify (const char * const str)
{
		assert (str != NULL);

		/* $GP - NMEA GPS-source strings only */
		if (str [0] != '$' || str [1] != 'G' || str [2] != 'P')
		{
				/*XXX: why does this fault under V2.4 and cause a lock up?!
				DPRINTF (("gps_nmea_decode: not a '$GP' NMEA sentence: '%s'\n", str));*/
				return gps_nmea_sentence_type_none;
		}

		/* if a valid NMEA string, then verify the checksum */
		if (gps_nmea_decode_verify_checksum (str) == FALSE)
		{
				DPRINTF (("gps_nmea_decode: checksum verification failed\n"));
				return gps_nmea_sentence_type_none;
		}

		/* now identify the specific type */
		GPS_NMEA_MATCH (str, 'G','G','A', gps_nmea_sentence_type_gga);
		GPS_NMEA_MATCH (str, 'G','L','L', gps_nmea_sentence_type_gll);
		GPS_NMEA_MATCH (str, 'G','S','A', gps_nmea_sentence_type_gsa);
		GPS_NMEA_MATCH (str, 'G','S','V', gps_nmea_sentence_type_gsv);
		GPS_NMEA_MATCH (str, 'R','M','C', gps_nmea_sentence_type_rmc);
		GPS_NMEA_MATCH (str, 'V','T','G', gps_nmea_sentence_type_vtg);
		GPS_NMEA_MATCH (str, 'Z','D','A', gps_nmea_sentence_type_zda);

		return gps_nmea_sentence_type_oth;
}

static boolean gps_nmea_decode_content (gps_nmea_sentence_t * const state, const gps_nmea_sentence_type_t type, const char * const str)
{
		assert (state != NULL);
		assert (type != gps_nmea_sentence_type_none);
		assert (str != NULL);

		/* GGA - gps fix data */
		if (type == gps_nmea_sentence_type_gga)
		{
				state->type = type;
				if (gps_nmea_decode_GGA (&state->stru.gga, str) == FALSE)
				{
						DPRINTF (("gps_nmea_decode: GGA decode failed\n"));
						return FALSE;
				}
		}

		/* RMC - recommended minimum GNSS data */
		else if (type == gps_nmea_sentence_type_rmc)
		{
				state->type = type;
				if (gps_nmea_decode_RMC (&state->stru.rmc, str) == FALSE)
				{
						DPRINTF (("gps_nmea_decode: RMC decode failed\n"));
						return FALSE;
				}
		}

		/* ZDA - time & date */
		else if (type == gps_nmea_sentence_type_zda)
		{
				state->type = type;
				if (gps_nmea_decode_ZDA (&state->stru.zda, str) == FALSE)
				{
						DPRINTF (("gps_nmea_decode: ZDA decode failed\n"));
						return FALSE;
				}
		}

		else
		{
				return FALSE;
		}

		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

static unsigned long gps_format_output_counter;
static unsigned long gps_format_output_interval;
static gps_nmea_sentence_type_t gps_format_nmea_sentences;
static gps_nmea_sentence_t gps_format_state_nmea;

typedef enum {
		GPS_FORMAT_NONE,
		GPS_FORMAT_RAW,
		GPS_FORMAT_NMEA,
		GPS_FORMAT_KML,
		GPS_FORMAT_CSV,
		GPS_FORMAT_TIME
} GPS_FORMAT_T;

typedef enum {
		GPS_FORMAT_CSV_TYPE_LAT = 0,
		GPS_FORMAT_CSV_TYPE_LON,
		GPS_FORMAT_CSV_TYPE_ALT,
		GPS_FORMAT_CSV_TYPE_TIM,
		GPS_FORMAT_CSV_TYPE_DAT,
		GPS_FORMAT_CSV_TYPE_MAX
} GPS_FORMAT_CSV_TYPE_T;

typedef struct {
		GPS_FORMAT_CSV_TYPE_T types [(int) GPS_FORMAT_CSV_TYPE_MAX + 1];
		boolean binary;
} GPS_FORMAT_CONFIG_CSV_T;

typedef union {
		GPS_FORMAT_CONFIG_CSV_T csv;
} GPS_FORMAT_CONFIG_T;

static GPS_FORMAT_T gps_format_type;
static GPS_FORMAT_CONFIG_T gps_format_config;

static unsigned int gps_format_encode_latitude (unsigned char * const data, const unsigned int offset, const signed char sign, const unsigned char major, const unsigned long minor, const boolean binary)
{
		unsigned int offs = offset;

		assert (data != NULL);

		if (!binary)
		{
				char temp [12];
				if (sign < (signed char) 0) data [offs++] = '-';
				offs += util_stratol ((char *)&data [offs], (signed long) major);
				offs += util_strcpy ((char *)&data [offs], ".");
				(void) util_stratol (temp, (signed long) minor);
				offs += util_strpad ((char *)&data [offs], temp, '0', (unsigned int) 6);
		}
		else
		{
				static signed char sign_prev = (signed char) -2;
				static unsigned char major_prev = (unsigned char) 0;
				static unsigned long minor_prev = (unsigned long) 0;
				signed long minor_diff = (signed long) minor - (signed long) minor_prev;
				if (sign == sign_prev && major == major_prev && UTIL_ABS (minor_diff) < (signed long) (1<<8))
				{
						offs = util_binpack (data, offs, (unsigned long) 1, (unsigned int) 1); /* relative */
						offs = util_binpack (data, offs, (minor_diff < (signed long) 0) ? (unsigned long) 1 : (unsigned long) 0, (unsigned int) 1);
						offs = util_binpack (data, offs, (unsigned long) UTIL_ABS (minor_diff), (unsigned int) 8);
						minor_prev = minor;
				}
				else
				{
						offs = util_binpack (data, offs, (unsigned long) 0, (unsigned int) 1); /* absolute */
						offs = util_binpack (data, offs, (sign < (signed char) 0) ? (unsigned long) 1 : (unsigned long) 0, (unsigned int) 1);
						offs = util_binpack (data, offs, (unsigned long) major, (unsigned int) 7); /* max 90 */
						offs = util_binpack (data, offs, minor, (unsigned int) 20); /* max 6 decimal places, i.e. 999999 */
						sign_prev = sign;
						major_prev = major;
						minor_prev = minor;
				}
		}
		return offs;
}

static unsigned int gps_format_encode_longitude (unsigned char * const data, const unsigned int offset, const signed char sign, const unsigned char major, const unsigned long minor, const boolean binary)
{
		unsigned int offs = offset;

		assert (data != NULL);

		if (!binary)
		{
				char temp [12];
				if (sign < (signed char) 0) data [offs++] = '-';
				offs += util_stratol ((char *)&data [offs], (signed long) major);
				offs += util_strcpy ((char *)&data [offs], ".");
				(void) util_stratol (temp, (signed long) minor);
				offs += util_strpad ((char *)&data [offs], temp, '0', (unsigned int) 6);
		}
		else
		{
				static signed char sign_prev = (signed char) -2;
				static unsigned char major_prev = (unsigned char) 0;
				static unsigned long minor_prev = (unsigned long) 0;
				signed long minor_diff = (signed long) minor - (signed long) minor_prev;
				if (sign == sign_prev && major == major_prev && UTIL_ABS (minor_diff) < (signed long) (1<<9))
				{
						offs = util_binpack (data, offs, (unsigned long) 1, (unsigned int) 1); /* relative */
						offs = util_binpack (data, offs, (minor_diff < (signed long) 0) ? (unsigned long) 1 : (unsigned long) 0, (unsigned int) 1);
						offs = util_binpack (data, offs, (unsigned long) UTIL_ABS (minor_diff), (unsigned int) 9);
						minor_prev = minor;
				}
				else
				{
						offs = util_binpack (data, offs, (unsigned long) 0, (unsigned int) 1); /* absolute */
						offs = util_binpack (data, offs, (sign < (signed char) 0) ? (unsigned long) 1 : (unsigned long) 0, (unsigned int) 1);
						offs = util_binpack (data, offs, (unsigned long) major, (unsigned int) 8); /* max 180 */
						offs = util_binpack (data, offs, minor, (unsigned int) 20); /* max 6 decimal places, i.e. 999999 */
						sign_prev = sign;
						major_prev = major;
						minor_prev = minor;
				}
		}
		return offs;
}

static unsigned int gps_format_encode_altitude (unsigned char * const data, const unsigned int offset, const signed short alt, const boolean binary)
{
		unsigned int offs = offset;

		assert (data != NULL);

		if (!binary)
		{
				offs += util_stratol ((char *)&data [offs], (signed long) alt);
		}
		else
		{
				static signed short alt_prev = (signed short) -(1<<15);
				signed short alt_diff = alt - alt_prev;
				if (UTIL_ABS (alt_diff) < (signed short) (1<<1))
				{
						offs = util_binpack (data, offs, (unsigned long) 1, (unsigned int) 1); /* relative */
						offs = util_binpack (data, offs, (alt_diff < (signed short) 0) ? (unsigned long) 1 : (unsigned long) 0, (unsigned int) 1);
						offs = util_binpack (data, offs, (unsigned long) UTIL_ABS (alt_diff), (unsigned int) 1);
						alt_prev = alt;
				}
				else
				{
						offs = util_binpack (data, offs, (unsigned long) 0, (unsigned int) 1); /* absolute */
						offs = util_binpack (data, offs, (alt < (signed short) 0) ? (unsigned long) 1 : (unsigned long) 0, (unsigned int) 1);
						offs = util_binpack (data, offs, (unsigned long) UTIL_ABS (alt), (unsigned int) 15); /* max 32768 */
						alt_prev = alt;
				}
		}
		return offs;
}

static unsigned int gps_format_encode_time (unsigned char * const data, const unsigned int offset, const char * const utc, const boolean binary)
{
		unsigned int offs = offset;

		assert (data != NULL);

		if (!binary)
		{
				offs += util_strcpy ((char *)&data [offs], utc);
		}
		else
		{
				unsigned long secs = (unsigned long) ( ((unsigned long)(((utc [0]-'0')*(char)10)+(utc [1]-'0')) * (unsigned long)(60*60)) + /* hours */
									 ( (unsigned long) (((utc [2]-'0')*(char)10)+(utc [3]-'0')) * (unsigned long)(60)) + /* minutes */
									 ( (unsigned long) (((utc [4]-'0')*(char)10)+(utc [5]-'0'))) );
				static unsigned long secs_prev = (unsigned long) (1<<31);
				if (secs > secs_prev && (secs - secs_prev) < (unsigned long) (1<<1))
				{
						offs = util_binpack (data, offs, (unsigned long) 1, (unsigned int) 1); /* relative */
						offs = util_binpack (data, offs, (unsigned long) (secs - secs_prev), (unsigned int) 1);
						secs_prev = secs;
				}
				else
				{
						offs = util_binpack (data, offs, (unsigned long) 0, (unsigned int) 1); /* absolute */
						offs = util_binpack (data, offs, secs, (unsigned int) 17); /* max 86400 */
						secs_prev = secs;
				}
		}
		return offs;
}

static unsigned int gps_format_encode_date (unsigned char * const data, const unsigned int offset, const char * const yy, const char * const mm, const char * const dd, const boolean binary)
{
		unsigned int offs = offset;

		assert (data != NULL);

		if (!binary)
		{
				offs += util_strcpy ((char *)&data [offs], dd);
				offs += util_strcpy ((char *)&data [offs], mm);
				offs += util_strcpy ((char *)&data [offs], (yy [0] > '7') ? "19" : "20");
				offs += util_strcpy ((char *)&data [offs], yy);
		}
		else
		{
				static char yy_prev [2] = { '\0', '\0' }, mm_prev [2] = { '\0', '\0' }, dd_prev [2] = { '\0', '\0' };
				if (dd [0] == dd_prev [0] && dd [1] == dd_prev [1] && mm [0] == mm_prev [0] && mm [1] == mm_prev [1] && yy [0] == yy_prev [0] && yy [1] == yy_prev [1])
				{
						offs = util_binpack (data, offs, (unsigned long) 1, (unsigned int) 1); /* relative */
				}
				else
				{
						unsigned int d = util_date_mjd (
									((unsigned int) ((yy [0] > '7' ? '1' : '2') - '0') * 1000) + ((unsigned int) ((yy [0] > '7' ? '9' : '0') - '0') * 100) +
									((unsigned int) (yy [0] - '0') * 10) + ((unsigned int) (yy [1] - '0')),
									(unsigned char) (((mm [0] - '0') * (char) 10) + (mm [1] - '0')),
									(unsigned char) (((dd [0] - '0') * (char) 10) + (dd [1] - '0')) );
						offs = util_binpack (data, offs, (unsigned long) 0, (unsigned int) 1); /* absolute */
						offs = util_binpack (data, offs, (unsigned long) d, (unsigned int) 14); /* max 16384 -> 44 years, 2024 */
						dd_prev [0] = dd [0]; dd_prev [1] = dd [1];
						mm_prev [0] = mm [0]; mm_prev [1] = mm [1];
						yy_prev [0] = yy [0]; yy_prev [1] = yy [1];
				}
		}
		return offs;
}

static unsigned int offset_rem = (unsigned int) 0;
static unsigned char offset_dat = (unsigned char) 0;

static unsigned int gps_format_encode_CSV (const gps_nmea_sentence_t * const content, unsigned char * const data, const unsigned int size,
												const GPS_FORMAT_CSV_TYPE_T * const typelist, const boolean binary)
{
		assert (content != NULL);
		assert (data != NULL);
		assert (size > 0);

		UNUSED (size);

		if (content->type == gps_nmea_sentence_type_gga)
		{
				unsigned int indax = 0, offset = 0;

				if (binary) { data [0] = offset_dat; offset = offset_rem; }

				while (indax < (unsigned int) ((int) GPS_FORMAT_CSV_TYPE_MAX + 1))
				{
/*@-loopswitchbreak@*/
						switch (typelist [indax++])
						{
								case GPS_FORMAT_CSV_TYPE_LAT:
										if (!binary && offset > 0) offset += util_strcat ((char *)&data [offset], ",");
										offset = gps_format_encode_latitude (data, offset, content->stru.gga.lat.sign, content->stru.gga.lat.major, content->stru.gga.lat.minor, binary);
										break;
								case GPS_FORMAT_CSV_TYPE_LON:
										if (!binary && offset > 0) offset += util_strcat ((char *)&data [offset], ",");
										offset = gps_format_encode_longitude (data, offset, content->stru.gga.lon.sign, content->stru.gga.lon.major, content->stru.gga.lon.minor, binary);
										break;
								case GPS_FORMAT_CSV_TYPE_ALT:
										if (!binary && offset > 0) offset += util_strcat ((char *)&data [offset], ",");
										offset = gps_format_encode_altitude (data, offset, content->stru.gga.alt, binary);
										break;
								case GPS_FORMAT_CSV_TYPE_TIM:
										if (!binary && offset > 0) offset += util_strcat ((char *)&data [offset], ",");
										offset = gps_format_encode_time (data, offset, content->stru.gga.utc, binary);
										break;
								case GPS_FORMAT_CSV_TYPE_DAT: /* ignore! */
										break;
								case GPS_FORMAT_CSV_TYPE_MAX:
										if (!binary) return offset + util_strcat ((char *) &data [offset], "\r\n");
										offset_rem = (offset & 0x07);
										offset_dat = data [offset >> 3];
										return (offset >> 3);
							default:
										return 0;
						}
/*@=loopswitchbreak@*/
				}
		}
		else if (content->type == gps_nmea_sentence_type_rmc)
		{
				unsigned int indax = 0, offset = 0;

				if (binary) { data [0] = offset_dat; offset = offset_rem; }

				while (indax < (unsigned int) ((int) GPS_FORMAT_CSV_TYPE_MAX + 1))
				{
/*@-loopswitchbreak@*/
						switch (typelist [indax++])
						{
								case GPS_FORMAT_CSV_TYPE_LAT:
										if (!binary && offset > 0) offset += util_strcat ((char *)&data [offset], ",");
										offset = gps_format_encode_latitude (data, offset, content->stru.rmc.lat.sign, content->stru.rmc.lat.major, content->stru.rmc.lat.minor, binary);
										break;
								case GPS_FORMAT_CSV_TYPE_LON:
										if (!binary && offset > 0) offset += util_strcat ((char *)&data [offset], ",");
										offset = gps_format_encode_longitude (data, offset, content->stru.rmc.lon.sign, content->stru.rmc.lon.major, content->stru.rmc.lon.minor, binary);
										break;
								case GPS_FORMAT_CSV_TYPE_ALT: /* ignore! */
										break;
								case GPS_FORMAT_CSV_TYPE_TIM:
										if (!binary && offset > 0) offset += util_strcat ((char *)&data [offset], ",");
										offset = gps_format_encode_time (data, offset, content->stru.rmc.utc, binary);
										break;
								case GPS_FORMAT_CSV_TYPE_DAT:
										if (!binary && offset > 0) offset += util_strcat ((char *)&data [offset], ",");
										offset = gps_format_encode_date (data, offset, content->stru.rmc.yy, content->stru.rmc.mm, content->stru.rmc.dd, binary);
										break;
								case GPS_FORMAT_CSV_TYPE_MAX:
										if (!binary) return offset + util_strcat ((char *) &data [offset], "\r\n");
										offset_rem = (offset & 0x07);
										offset_dat = data [offset >> 3];
										return (offset >> 3);
							default:
										return 0;
						}
/*@=loopswitchbreak@*/
				}
		}

		return 0;
}

/* ----------------------------------------------------------------------------------------------------*/

static unsigned int gps_format_RAW (unsigned char * const data, const unsigned int size)
{
		unsigned char * data_ptr = data;

		assert (data != NULL);
		assert (size > 0);

		UNUSED (size);

		while (*data_ptr != (unsigned char) '\0' && (*data_ptr != (unsigned char) '\r' && *data_ptr != (unsigned char) '\n'))
				data_ptr++;
		*data_ptr++ = (unsigned char) '\r';
		*data_ptr++ = (unsigned char) '\n';
		*data_ptr++ = (unsigned char) '\0';
		return util_strlen ((char *)data);
}

static unsigned int gps_format_NMEA (unsigned char * const data, const unsigned int size)
{
		unsigned char * data_ptr = data;

		assert (data != NULL);
		assert (size > 0);

		UNUSED (size);

		while (*data_ptr != (unsigned char) '\0' && (*data_ptr != (unsigned char) '\r' && *data_ptr != (unsigned char) '\n'))
				data_ptr++;
		*data_ptr++ = (unsigned char) '\r';
		*data_ptr++ = (unsigned char) '\n';
		*data_ptr++ = (unsigned char) '\0';
		return util_strlen ((char *)data);
}

static unsigned int gps_format_KML (const gps_nmea_sentence_t * const content, unsigned char * const data, const unsigned int size)
{
		const GPS_FORMAT_CSV_TYPE_T typelist [] = { GPS_FORMAT_CSV_TYPE_LON, GPS_FORMAT_CSV_TYPE_LAT, GPS_FORMAT_CSV_TYPE_ALT, GPS_FORMAT_CSV_TYPE_MAX };

		assert (content != NULL);
		assert (data != NULL);
		assert (size > 0);

		return gps_format_encode_CSV (content, data, size, &typelist [0], FALSE);
}

static unsigned int gps_format_CSV (const gps_nmea_sentence_t * const content, unsigned char * const data, const unsigned int size)
{
		assert (content != NULL);
		assert (data != NULL);
		assert (size > 0);

		return gps_format_encode_CSV (content, data, size, &gps_format_config.csv.types [0], gps_format_config.csv.binary);
}

static unsigned int gps_format_TIME (const gps_nmea_sentence_t * const content, unsigned char * const data, const unsigned int size)
{
		assert (content != NULL);
		assert (data != NULL);
		assert (size > 0);

		if (content->type == gps_nmea_sentence_type_rmc)
		{
				/* XXX need caching ... */
				unsigned char * dptr = data;
				const char * const dd = content->stru.rmc.dd;
				const char * const mm = content->stru.rmc.mm;
				const char * const yy = content->stru.rmc.yy;

				/*Thu Nov 24 18:22:48 1986*/

				dptr += util_strcpy ((char *) dptr, util_date_str_dayofweek (
							((unsigned int) ((yy [0] > '7' ? '1' : '2') - '0') * 1000) + ((unsigned int) ((yy [0] > '7' ? '9' : '0') - '0') * 100) +
							((unsigned int) (yy [0] - '0') * 10) + ((unsigned int) (yy [1] - '0')),
							(unsigned char) (((mm [0] - '0') * (char) 10) + (mm [1] - '0')),
							(unsigned char) (((dd [0] - '0') * (char) 10) + (dd [1] - '0')) ));
				*dptr++ = (unsigned char) ' ';
				dptr += util_strcpy ((char *) dptr, util_date_str_month (
							(unsigned char) (((mm [0] - '0') * (char) 10) + (mm [1] - '0')) ));
				*dptr++ = (unsigned char) ' ';
				*dptr++ = (unsigned char) (dd [0] == '0' ? ' ' : dd [0]);
				*dptr++ = (unsigned char) (dd [1]);
				*dptr++ = (unsigned char) ' ';
				*dptr++ = (unsigned char) (content->stru.rmc.utc [0] == '0' ? ' ' : content->stru.rmc.utc [0]);
				*dptr++ = (unsigned char) (content->stru.rmc.utc [1]);
				*dptr++ = (unsigned char) ':';
				*dptr++ = (unsigned char) (content->stru.rmc.utc [2]);
				*dptr++ = (unsigned char) (content->stru.rmc.utc [3]);
				*dptr++ = (unsigned char) ':';
				*dptr++ = (unsigned char) (content->stru.rmc.utc [4]);
				*dptr++ = (unsigned char) (content->stru.rmc.utc [5]);
				*dptr++ = (unsigned char) ' ';
				*dptr++ = (unsigned char) (yy [0] > '7' ? '1' : '2');
				*dptr++ = (unsigned char) (yy [0] > '7' ? '9' : '0');
				*dptr++ = (unsigned char) (yy [0]);
				*dptr++ = (unsigned char) (yy [1]);
				*dptr++ = (unsigned char) '\r';
				*dptr++ = (unsigned char) '\n';
				*dptr++ = (unsigned char) '\0';
				return (unsigned int) (4+4+3+3+3+3+4+2);
		}

		return (unsigned int) 0;
}

/* ----------------------------------------------------------------------------------------------------*/

unsigned int gps_format_process (unsigned char * str, const unsigned int size)
{
		gps_nmea_sentence_type_t type;

		assert (str != NULL);

		if (gps_format_type == GPS_FORMAT_RAW)
				return gps_format_RAW (str, size);

		while (str [0] != (unsigned char) '\0' && str [0] != (unsigned char) '$')
				str++;

		if ((type = gps_nmea_decode_identify ((char *) str)) == gps_nmea_sentence_type_none)
				return 0;

		if (gps_format_nmea_sentences != gps_nmea_sentence_type_all &&
					(gps_format_nmea_sentences & type) == gps_nmea_sentence_type_none)
				return 0;

		if (++gps_format_output_counter < gps_format_output_interval)
				return 0;
		gps_format_output_counter = 0;

		if (gps_format_type == GPS_FORMAT_NMEA)
				return gps_format_NMEA (str, size);

		if (gps_nmea_decode_content (&gps_format_state_nmea, type, (char *) str) == FALSE)
				return 0;

		if (gps_process_handle (&gps_format_state_nmea) == FALSE)
				return 0;

		str [0] = '\0';
		if (gps_format_type == GPS_FORMAT_KML)
				return gps_format_KML (&gps_format_state_nmea, str, size);
		else if (gps_format_type == GPS_FORMAT_CSV)
				return gps_format_CSV (&gps_format_state_nmea, str, size);
		else if (gps_format_type == GPS_FORMAT_TIME)
				return gps_format_TIME (&gps_format_state_nmea, str, size);

		/* should never occur ... */
		assert (FALSE);
		return 0;
}

boolean gps_format_init (const gps_config_format_t * const config)
{
		assert (config != NULL);
		assert (config->format == gps_config_format_raw ||
				config->format == gps_config_format_nmea ||
				config->format == gps_config_format_kml ||
				config->format == gps_config_format_csv ||
				config->format == gps_config_format_time);

		/* configure format */
		DPRINTF (("gps_format = %d\n", (int) config->format));

		gps_format_nmea_sentences = gps_nmea_sentence_type_none;

		if (config->format == gps_config_format_raw)
		{
				gps_format_type = GPS_FORMAT_RAW;
		}
		else if (config->format == gps_config_format_nmea)
		{
				assert (config->nmea_sentences != NULL);

				gps_format_type = GPS_FORMAT_NMEA;

				DPRINTF (("format_nmea_sentences = %s\n", config->nmea_sentences));

				if (util_strcmpix ("gga", config->nmea_sentences, ',')) gps_format_nmea_sentences |= gps_nmea_sentence_type_gga;
				if (util_strcmpix ("gll", config->nmea_sentences, ',')) gps_format_nmea_sentences |= gps_nmea_sentence_type_gll;
				if (util_strcmpix ("gsa", config->nmea_sentences, ',')) gps_format_nmea_sentences |= gps_nmea_sentence_type_gsa;
				if (util_strcmpix ("gsv", config->nmea_sentences, ',')) gps_format_nmea_sentences |= gps_nmea_sentence_type_gsv;
				if (util_strcmpix ("rmc", config->nmea_sentences, ',')) gps_format_nmea_sentences |= gps_nmea_sentence_type_rmc;
				if (util_strcmpix ("vtg", config->nmea_sentences, ',')) gps_format_nmea_sentences |= gps_nmea_sentence_type_vtg;
				if (util_strcmpix ("zda", config->nmea_sentences, ',')) gps_format_nmea_sentences |= gps_nmea_sentence_type_zda;
				if (util_strcmpix ("***", config->nmea_sentences, ',')) gps_format_nmea_sentences |= gps_nmea_sentence_type_all;
		}
		else if (config->format == gps_config_format_kml)
		{
				gps_format_type = GPS_FORMAT_KML;

				gps_format_nmea_sentences |= gps_nmea_sentence_type_gga;
		}
		else if (config->format == gps_config_format_csv)
		{
				int indax = 0;
				char type [4];
				const char * pntr = config->csv_content;

				assert (config->csv_content != NULL);

				gps_format_type = GPS_FORMAT_CSV;

				while ((pntr = util_csv_field_extract (pntr, type, (unsigned char) sizeof (type))) != NULL)
				{
						if (util_strcmpi (type, "lat")) {
							gps_format_config.csv.types [indax++] = GPS_FORMAT_CSV_TYPE_LAT;
							gps_format_nmea_sentences |= gps_nmea_sentence_type_gga;
						} else if (util_strcmpi (type, "lon")) {
							gps_format_config.csv.types [indax++] = GPS_FORMAT_CSV_TYPE_LON;
							gps_format_nmea_sentences |= gps_nmea_sentence_type_gga;
						} else if (util_strcmpi (type, "alt")) {
							gps_format_config.csv.types [indax++] = GPS_FORMAT_CSV_TYPE_ALT;
							gps_format_nmea_sentences |= gps_nmea_sentence_type_gga;
						} else if (util_strcmpi (type, "tim")) {
							gps_format_config.csv.types [indax++] = GPS_FORMAT_CSV_TYPE_TIM;
							gps_format_nmea_sentences |= gps_nmea_sentence_type_gga;
						} else if (util_strcmpi (type, "dat")) {
							gps_format_config.csv.types [indax++] = GPS_FORMAT_CSV_TYPE_DAT;
							gps_format_nmea_sentences |= gps_nmea_sentence_type_zda;
						}
				}

				gps_format_config.csv.types [indax] = GPS_FORMAT_CSV_TYPE_MAX;
				gps_format_config.csv.binary = config->csv_binary;
		}
		else if (config->format == gps_config_format_time)
		{
				gps_format_type = GPS_FORMAT_TIME;

				gps_format_nmea_sentences |= gps_nmea_sentence_type_rmc;
		}

		return TRUE;
}

unsigned int gps_format_term (unsigned char * str, const unsigned int size)
{
		UNUSED (size);

		if (gps_format_type == GPS_FORMAT_CSV && gps_format_config.csv.binary)
		{
				if (offset_rem > 0)
				{
						str [0] = offset_dat;
						return (unsigned int) 1;
				}
		}

		return (unsigned int) 0;
}

/* ----------------------------------------------------------------------------------------------------*/

#ifdef TEST_ENABLED

test_result_t gps_format_test (void) /* XXX */
{
		return TEST_RESULT_OKAY;
}

#endif

/* ----------------------------------------------------------------------------------------------------*/

