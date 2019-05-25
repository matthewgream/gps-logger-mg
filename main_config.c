
/* ----------------------------------------------------------------------------------------------------*/
/*
	main_config.c: gps logger main configuration data.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#include "common.h"
#include "mod_util.h"
#include "mod_lpc21XXhal.h"
#include "mod_config.h"
#include "mod_gps.h"
#include "main_config.h"

/* ----------------------------------------------------------------------------------------------------*/

#define CFG_FILE_NAME	"GPSCONFG.TXT"
#define CFG_FILE_HEAD	"# gps_logger_mg v" VERSION ": firmware configuration\n" \
						"# default values generated automatically, edit as required\n"
#define CFG_FILE_FOOT	"\n# end\n"

static boolean cfg_call__debug (const char * const data)
{
		return util_strcmpix (data, DEBUG_MODE_LIST, '|');
}

static boolean cfg_call__interval (const signed long data)
{
		return (data > (signed long) 0) ? TRUE : FALSE;
}

static boolean cfg_call__format (const char * const data)
{
		return util_strcmpix (data, GPS_FORMAT_LIST, '|');
}

static boolean cfg_call__format_nmea_sentences (const char * const data)
{
		return util_strcmpix (data, GPS_FORMAT_NMEA_SENTENCE_LIST, '|');
}

static boolean cfg_call__format_csv_content (const char * const data)
{
		return util_strcmpix (data, GPS_FORMAT_CSV_ELEMENT_LIST, '|');
}

static boolean cfg_call__format_csv_encoding (const char * const data)
{
		return util_strcmpix (data, GPS_FORMAT_CSV_ENCODING_LIST, '|');
}

static boolean cfg_call__mode (const char * const data)
{
		return util_strcmpix (data, GPS_MODE_LIST, '|');
}

static boolean cfg_call__mode_file_buffer_size (const signed long data)
{
		return (data == (signed long) 0 || (data >= (signed long) 64 && data <= (signed long) util_buffer_len ())) ? TRUE : FALSE;
}

static boolean cfg_call__mode_file_buffer_timeout (const signed long data)
{
		return (data >= (signed long) 0) ? TRUE : FALSE;
}

static boolean cfg_call__mode_pass_serial_speed (const signed long data)
{
		return lpc21XX_uart_baud_supported ((unsigned int) data);
}

/*@-stringliteralsmaller@*/
static cfg_data_string_t  cfg_data_debug = DEBUG_MODE_DEFAULT;
static cfg_data_integer_t cfg_data_interval = (signed long) 1;
static cfg_data_string_t  cfg_data_format = GPS_FORMAT_DEFAULT;
static cfg_data_string_t  cfg_data_format_nmea_sentences = GPS_FORMAT_NMEA_SENTENCE_DEFAULT;
static cfg_data_string_t  cfg_data_format_csv_content = GPS_FORMAT_CSV_ELEMENT_DEFAULT;
static cfg_data_string_t  cfg_data_format_csv_encoding = GPS_FORMAT_CSV_ENCODING_DEFAULT;
static cfg_data_string_t  cfg_data_mode = GPS_MODE_DEFAULT;
static cfg_data_integer_t cfg_data_mode_file_buffer_size = (signed long) 512 /* CARD_SECTOR_SIZE */;
static cfg_data_integer_t cfg_data_mode_file_buffer_timeout_normal = (signed long) 0;
static cfg_data_integer_t cfg_data_mode_file_buffer_timeout_quiet = (signed long) 0;
static cfg_data_integer_t cfg_data_mode_pass_serial_speed = (signed long) 4800;
static cfg_data_boolean_t cfg_data_compress = FALSE;
static cfg_data_boolean_t cfg_data_update_rtc = FALSE;
/*@=stringliteralsmaller@*/

static const cfg_item_t cfg_item_list [] =
{
		{ "debug", "debug options", DEBUG_MODE_LIST " (none, output trace or diagnostics self-test)",
			t_string, CFG_FLAG_FORMAT_NONE, .call.c_string = cfg_call__debug, .data.d_string = &cfg_data_debug },
		{ "interval", "output interval", "1 ... [# secs, #m mins, #h hours, #d days]",
			t_integer, CFG_FLAG_FORMAT_TIMEPERIOD, .call.c_integer = cfg_call__interval, .data.d_integer = &cfg_data_interval },
		{ "format", "format for NMEA GPS output data", GPS_FORMAT_LIST " (raw data, NMEA data, Google Earth KML, Comma Separated Variables, Timestring)",
			t_string, CFG_FLAG_FORMAT_NONE, .call.c_string = cfg_call__format, .data.d_string = &cfg_data_format },
		{ "format_nmea_sentences", "GPS NMEA sentences to select with NMEA format (comma separated)", GPS_FORMAT_NMEA_SENTENCE_LIST " (supported NMEA sentences, or '***' as a wildcard for all)",
			t_string, CFG_FLAG_FORMAT_CSV, .call.c_string = cfg_call__format_nmea_sentences, .data.d_string = &cfg_data_format_nmea_sentences },
		{ "format_csv_content", "content of each entry for CSV format output", GPS_FORMAT_CSV_ELEMENT_LIST " (refer to documentation)",
			t_string, CFG_FLAG_FORMAT_CSV, .call.c_string = cfg_call__format_csv_content, .data.d_string = &cfg_data_format_csv_content },
		{ "format_csv_encoding", "encoding for CSV format output", GPS_FORMAT_CSV_ENCODING_LIST " (text or binary)",
			t_string, CFG_FLAG_FORMAT_NONE, .call.c_string = cfg_call__format_csv_encoding, .data.d_string = &cfg_data_format_csv_encoding },
		{ "mode", "operating mode", GPS_MODE_LIST " (file store on card, pass through serial-port -- use one or both)",
			t_string, CFG_FLAG_FORMAT_CSV, .call.c_string = cfg_call__mode, .data.d_string = &cfg_data_mode },
		{ "mode_file_buffer_size", "file buffer size (number of bytes of RAM to use to cache entries before file writes)",
					"0, 64 ... " util_xstr (UTIL_BUFFER_SIZE) " bytes",
			t_integer, CFG_FLAG_FORMAT_BASE2BYTES, .call.c_integer = cfg_call__mode_file_buffer_size, .data.d_integer = &cfg_data_mode_file_buffer_size },
		{ "mode_file_buffer_timeout_normal", "file buffer normal timeout (max time before forcing file write)",
					"0 (disabled) ... [# secs, #m mins, #h hours, #d days]",
			t_integer, CFG_FLAG_FORMAT_TIMEPERIOD, .call.c_integer = cfg_call__mode_file_buffer_timeout, .data.d_integer = &cfg_data_mode_file_buffer_timeout_normal },
		{ "mode_file_buffer_timeout_quiet", "file buffer quiet timeout (max time with no data received before forcing file write)",
					"0 (disabled) ... [# secs, #m mins, #h hours, #d days]",
			t_integer, CFG_FLAG_FORMAT_TIMEPERIOD, .call.c_integer = cfg_call__mode_file_buffer_timeout, .data.d_integer = &cfg_data_mode_file_buffer_timeout_quiet },
		{ "mode_pass_serial_speed", "pass output serial speed", "1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200 (bits per second, 8N1)",
			t_integer, CFG_FLAG_FORMAT_NONE, .call.c_integer = cfg_call__mode_pass_serial_speed, .data.d_integer = &cfg_data_mode_pass_serial_speed },
		{ "compress", "perform compression (lzari based) on output data (reduces buffer size by 8K)", NULL,
			t_boolean, CFG_FLAG_FORMAT_NONE, .call.c_boolean = NULL, .data.d_boolean = &cfg_data_compress },
		{ "update_rtc", "update the RTC from GPS (if unset, or until first update, clock will read 1980/01/01 00:00:00)", NULL,
			t_boolean, CFG_FLAG_FORMAT_NONE, .call.c_boolean = NULL, .data.d_boolean = &cfg_data_update_rtc }
};

static const cfg_config_t cfg_config =
{
		CFG_FILE_NAME,
		CFG_FILE_HEAD,
		CFG_FILE_FOOT,
		cfg_item_list,
		(unsigned char) (sizeof (cfg_item_list) / sizeof (cfg_item_list [0]))
};

/* ----------------------------------------------------------------------------------------------------*/

/*@-compmempass@*/
const cfg_config_t* main_cfg_config (void)
{
		return &cfg_config;
}
/*@=compmempass@*/

/* ----------------------------------------------------------------------------------------------------*/

#ifdef TEST_ENABLED

test_result_t main_config_test (void)
{
		int i, n = (int) (sizeof (cfg_item_list) / sizeof (cfg_item_list [0]));

		DPRINTF (("main_config_test: %d configs\n", n));

		test_assert (main_cfg_config () != NULL);

		test_assert (cfg_config.filename != NULL && cfg_config.default_header != NULL && cfg_config.default_footer != NULL);
		test_assert (cfg_config.item_list != NULL && cfg_config.item_size > (unsigned char) 0);

		for (i = 0; i < n; i++)
		{
				const cfg_item_t * const cfg = &cfg_item_list [i];

				test_assert (cfg->name != NULL);
				test_assert (cfg->description_name != NULL);
				test_assert (cfg->type == t_boolean || cfg->type == t_integer || cfg->type == t_string);
				if (cfg->type == t_boolean) {
					test_assert (cfg->flag == CFG_FLAG_FORMAT_NONE);
					test_assert (cfg->data.d_boolean != NULL);
				} else if (cfg->type == t_integer) { 
					test_assert (cfg->flag == CFG_FLAG_FORMAT_NONE || cfg->flag == CFG_FLAG_FORMAT_TIMEPERIOD || cfg->flag == CFG_FLAG_FORMAT_BASE2BYTES);
					test_assert (cfg->call.c_integer != NULL);
					test_assert (cfg->data.d_integer != NULL);
				} else if (cfg->type == t_string) {
					test_assert (cfg->flag == CFG_FLAG_FORMAT_NONE || cfg->flag == CFG_FLAG_FORMAT_CSV);
					test_assert (cfg->call.c_string != NULL);
					test_assert (cfg->data.d_string != NULL);
				}
		}

		return TEST_RESULT_OKAY;
}

#endif

/* ----------------------------------------------------------------------------------------------------*/

