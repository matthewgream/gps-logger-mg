
/* ----------------------------------------------------------------------------------------------------*/
/*
	main.c: gps logger main application configuration and co-ordination.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#include "common.h"
#include "mod_util.h"
#include "mod_lpc21XXhal.h"
#include "mod_filefatfs.h"
#include "mod_config.h"
#include "mod_command.h"
#include "mod_gps.h"

#include "main_config.h"
#include "main_command.h"
#include "main_debug.h"

#define cfg_data_get(c,n) (&(cfg_item_get(c,n)->data))

/* ----------------------------------------------------------------------------------------------------*/

static volatile boolean app_actv;
static void app_init (const int argc, const char ** const argv);
static void app_exec (void);
static void app_term (void);

/* ----------------------------------------------------------------------------------------------------*/

static void hndl_terminate (void)
{
		app_actv = FALSE;
}

/* ----------------------------------------------------------------------------------------------------*/

int main (const int argc, const char ** const argv)
{
		if (lpc21XX_init (argc, argv) == FALSE)
		{
				HALT (("main: failed - lpc21XX_init\n"));
		}

#ifdef DEBUG
		debug_init ();

		DPRINTF (("gps_logger_mg v" VERSION ": firmware executable (hardware: " HARDWARE ")\n"));
		DPRINTF ((COPYRIGHT "\n"));
#endif

		app_init (argc, argv);

		app_actv = TRUE;
		lpc21XX_shutdown_enable (hndl_terminate);
		lpc21XX_powerfail_enable (hndl_terminate);

		app_exec ();

		lpc21XX_powerfail_disable ();
		lpc21XX_shutdown_disable ();
		app_actv = FALSE;

		app_term ();

#ifdef DEBUG
		debug_term ();
#endif

		lpc21XX_down ();

		/*@notreached@*/ return 0;
}

/* ----------------------------------------------------------------------------------------------------*/

static void app_init (const int argc, const char ** const argv)
{
		fat_config_t fat_config;
		fat_error_t fat_result;
		gps_config_t gps_config;
		const cfg_config_t * cfg_config = main_cfg_config ();
		const cmd_config_t * cfg_command = main_cfg_command ();

		DPRINTF (("app_init\n"));

		if ((fat_result = fat_init (&fat_config)) != FAT_ERR_NONE)
		{
				HALT (("app_init: failed - fat_init, error=%d\n", fat_result));
		}

		if (cfg_init (cfg_config, argc, argv) == FALSE)
		{
				HALT (("app_init: failed - cfg_init\n"));
		}

		if (cmd_init (cfg_command) == FALSE)
		{
				HALT (("app_init: failed - cmd_init\n"));
		}

		/* XXX can this be pushed out of app_* */
#ifdef DEBUG
		if (debug_mode (*cfg_data_get (cfg_config, "debug")->d_string) == FALSE)
		{
				HALT (("app_init: failed - debug_mode\n"));
		}
#endif

		gps_config.input.interval = (unsigned long) *cfg_data_get (cfg_config, "interval")->d_integer;

		if (util_strcmpi (*cfg_data_get (cfg_config, "format")->d_string, "raw"))
				gps_config.format.format = gps_config_format_raw;
		else if (util_strcmpi (*cfg_data_get (cfg_config, "format")->d_string, "nmea"))
				gps_config.format.format = gps_config_format_nmea;
		else if (util_strcmpi (*cfg_data_get (cfg_config, "format")->d_string, "kml"))
				gps_config.format.format = gps_config_format_kml;
		else if (util_strcmpi (*cfg_data_get (cfg_config, "format")->d_string, "csv"))
				gps_config.format.format = gps_config_format_csv;
		else /*if (util_strcmpi (*cfg_data_get (cfg_config, "format")->d_string, "time"))*/
				gps_config.format.format = gps_config_format_time;
		gps_config.format.nmea_sentences = *cfg_data_get (cfg_config, "format_nmea_sentences")->d_string;
		gps_config.format.csv_content = *cfg_data_get (cfg_config, "format_csv_content")->d_string;
		if (util_strcmpi (*cfg_data_get (cfg_config, "format_csv_encoding")->d_string, "binary"))
				gps_config.format.csv_binary = TRUE;
		else /*if (util_strcmpi (*cfg_data_get (cfg_config, "format_csv_encoding")->d_string, "text"))*/
				gps_config.format.csv_binary = FALSE;

		if (util_strcmpix ("pass", *cfg_data_get (cfg_config, "mode")->d_string, ',')) {
				gps_config.output.pass.enabled = TRUE;
				gps_config.output.pass.serial_speed = (unsigned int) *cfg_data_get (cfg_config, "mode_pass_serial_speed")->d_integer;
		} else {
				gps_config.output.pass.enabled = FALSE;
		}
		if (util_strcmpix ("file", *cfg_data_get (cfg_config, "mode")->d_string, ',')) {
				gps_config.output.file.enabled = TRUE;
				gps_config.output.file.buffer_size = (unsigned int) *cfg_data_get (cfg_config, "mode_file_buffer_size")->d_integer;
				gps_config.output.file.timeout_normal = (unsigned long) *cfg_data_get (cfg_config, "mode_file_buffer_timeout_normal")->d_integer;
				gps_config.output.file.timeout_quiet = (unsigned long) *cfg_data_get (cfg_config, "mode_file_buffer_timeout_quiet")->d_integer;
		} else {
				gps_config.output.file.enabled = FALSE;
		}
		gps_config.output.compress = *cfg_data_get (cfg_config, "compress")->d_boolean;

		gps_config.process.update_rtc = *cfg_data_get (cfg_config, "update_rtc")->d_boolean;

/*@-compdef@*/
		if (gps_init (&gps_config) == FALSE)
		{
				HALT (("app_init: failed - gps_init\n"));
		}
/*@=compdef@*/

		(void) fat_suspend ();
}

/* ----------------------------------------------------------------------------------------------------*/

static void app_exec (void)
{
		DPRINTF (("app_exec\n"));

		/* XXX: should be watchdogging in here, with say a 60 second period, and 30 second kicks
		 * problem is that we need to tune the length of the watchdog according to the gps period */
		/*@i@*/ while (app_actv == TRUE && util_scheduler_process () == TRUE)
		{
		}
}

/* ----------------------------------------------------------------------------------------------------*/

static void app_term (void)
{
		fat_error_t fat_result;

		DPRINTF (("app_term\n"));

		(void) fat_resume ();

		if (gps_term () == FALSE)
		{
				HALT (("app_term: failed - gps_term\n"));
		}

		if ((fat_result = fat_term ()) != FAT_ERR_NONE)
		{
				HALT (("app_term: failed - fat_term, error=%d\n", fat_result));
		}
}

/* ----------------------------------------------------------------------------------------------------*/
