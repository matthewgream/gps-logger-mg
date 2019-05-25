
/* ----------------------------------------------------------------------------------------------------*/
/*
	main_command.c: gps logger main command data.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#include "common.h"
#include "mod_util.h"
#include "mod_lpc21XXhal.h"
#include "mod_config.h"
#include "mod_command.h"
#include "main_command.h"
#include "main_config.h"

#include "mod_gpscapture.h"
#include "mod_filefatfs.h"
#include "mod_xferymodem.h"

/* ----------------------------------------------------------------------------------------------------*/

static boolean cmd_util_display (/*@notnull@*/ const char * const filename)
{
		fat_handle_t handle;
		char buffer [96]; unsigned int bufsiz;

		handle = fat_open (filename, FAT_OPT_READ);
		if (!FAT_HANDLE_VALID (handle))
				return FALSE;

		while (fat_read (handle, (unsigned char *) buffer, (unsigned int) (sizeof (buffer) - 1), &bufsiz) == FAT_ERR_NONE && bufsiz > 0)
		{
				buffer [bufsiz] = '\0';
				DPRINTF (("%s", buffer)); /*XXX*/
		}

		return fat_close (handle) == FAT_ERR_NONE ? TRUE : FALSE;
}

static boolean cmd_util_transfer (/*@notnull@*/ const char * const filename)
{
		UNUSED (filename);
		/*XXX*/
		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

#define CMD_TEXT_PROMPT "glc: "
#define CMD_TEXT_HELP "\
The command line interface must be enabled in the configuration, then after reset it\
will become active. If it is disabled in the configuration, then reset must occur to\
effect change. Note that any configuration change only has effect after reset.\
\
The interface provides a prompt labelled 'glc: ' (for 'gps logger command') at which\
commands (whether by user or another program) can be entered to control the device.\
Clear, delete and backspace characters are supported. All inputs are case insensitive.\
\
For help, use '?' for a command list, and 'help' for detailed command description.\
\
All commands may be used in a short or long form. In the long form, either the all,\
or any number of initial letters, in each word can be used as long as spacing is\
retained. In the short form, the initial letter of each word can be concatenated.\
The arguments have limited type checking and wildcard support (where noted). The\
allowable wildcards are '?' or '.' (match any single character) and '*' (match any\
sequence of characters). All commands must be terminated by a carraige return.\
\
For example, some commands with their long and short form equivalents are:\
- 'config clear', 'con cl', 'config c' (long forms); 'cc' (short form)\
- 'file transfer gps*.dat', 'f t gps*.dat' (long forms); 'ft gps*.dat' (short form)\
- 'reset', 'res', 'r' (long forms); 'r' (short form)\
\
Please refer to the manual for more detail about individual commands and features.\
"

/* ----------------------------------------------------------------------------------------------------*/

#ifdef DEBUG
static boolean cmd_call__stub (const cmd_arg_t* const arg)
{
		UNUSED (arg);

		return TRUE;
}
#endif

/* ----------------------------------------------------------------------------------------------------*/

static boolean cmd_call__version (const cmd_arg_t* const arg)
{
		UNUSED (arg);

		DPRINTF (("gps_logger_mg v" VERSION ": firmware executable (hardware: " HARDWARE ")\n")); /*XXX*/
		DPRINTF ((COPYRIGHT "\n")); /*XXX*/

		return TRUE;
}

static boolean cmd_call__diagnostics (const cmd_arg_t* const arg)
{
		UNUSED (arg);
		/*XXX*/
		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

/*@noreturn@*/ static boolean cmd_call__reset (const cmd_arg_t* const arg) __attribute__ ((noreturn));
static boolean cmd_call__reset (const cmd_arg_t* const arg)
{
		UNUSED (arg);

		lpc21XX_reset ();

		/*return TRUE;*/
}

/* ----------------------------------------------------------------------------------------------------*/

static boolean cmd_call__date_get (const cmd_arg_t* const arg)
{
		util_clock_buf_t cb;

		UNUSED (arg);

        (void) util_clock_get (cb);
		DPRINTF (("%02d:%02d:%02d %02d/%02d/%04d\n", cb [3], cb [4], cb [5], cb [2], cb [1], (int) cb [0] + 1980)); /*XXX*/

		return TRUE;
}

static void digits_set (/*@notnull@*/ /*@out@*/ unsigned char * const b, /*@notnull@*/ const char * const n)
{
		const char t [3] = { n [0], n [1], '\0' };
		(*b) = (unsigned char) (util_strtol (t) & (unsigned long) 0xFF);
}

static boolean cmd_call__date_set (const cmd_arg_t* const arg)
{
		util_clock_buf_t cb;
		unsigned char yh, yl;
		int t;

		if (arg == NULL || arg [0] == NULL)
				return FALSE;

		for (t = 0; t < 4+2+2 + 2+2+2; t++)
				if (!UTIL_ISDIGIT (arg [0][t]))
						return FALSE;

		digits_set (&yh, &arg [0][0]);
		digits_set (&yl, &arg [0][2]);
		if (yh == (unsigned char) 19 && yl >= (unsigned char) 80)
				cb [0] = (unsigned char) (yl - (unsigned char) 80);
		else if (yh == (unsigned char) 20 && yl <= (unsigned char) 99)
				cb [0] = (unsigned char) (yl + (unsigned char) 20);
		else
				return FALSE;

		digits_set (&cb [1], &arg [0][4]);
		digits_set (&cb [2], &arg [0][6]);
		digits_set (&cb [3], &arg [0][8]);
		digits_set (&cb [4], &arg [0][10]);
		digits_set (&cb [5], &arg [0][12]);

		return util_clock_set (cb);
}

/* ----------------------------------------------------------------------------------------------------*/

static boolean cmd_call__config_get (const cmd_arg_t* const arg)
{
		const cfg_item_t * item;

		if (arg == NULL || arg [0] == NULL)
				return FALSE;

		if ((item = cfg_item_get (main_cfg_config (), arg [0])) == NULL)
				return FALSE;

		if (item->type == t_boolean) {
				DPRINTF (("%s = %s\n", item->name, (*(item->data.d_boolean) == TRUE) ? "true" : "false"));
		} else if (item->type == t_integer) {
				DPRINTF (("%s = %ld\n", item->name, *(item->data.d_integer)));
		} else if (item->type == t_string) {
				DPRINTF (("%s = %s\n", item->name, *(item->data.d_string)));
		}
	
		return TRUE;
}

static boolean cmd_call__config_set (const cmd_arg_t* const arg)
{
		const cfg_item_t * item;

		if (arg == NULL || arg [0] == NULL || arg [1] == NULL)
				return FALSE;

		if (cfg_item_set (main_cfg_config (), arg [0], arg [1]) == FALSE)
				return FALSE;

		return TRUE;
}

static boolean cmd_call__config_clear (const cmd_arg_t* const arg)
{
		UNUSED (arg);

		return fat_unlink (cfg_filename (main_cfg_config ())) == FAT_ERR_NONE ? TRUE : FALSE;
}

static boolean cmd_call__config_display (const cmd_arg_t* const arg)
{
		UNUSED (arg);

		return cmd_util_display (cfg_filename (main_cfg_config ()));
}

static boolean cmd_call__config_transfer (const cmd_arg_t* const arg)
{
		UNUSED (arg);

		return cmd_util_transfer (cfg_filename (main_cfg_config ()));
}

/* ----------------------------------------------------------------------------------------------------*/

static boolean valid_file (/*@notnull@*/ const char * const filename)
{
		return (util_strlen (filename) == (unsigned int) 12 &&
				filename [0] == 'G' && filename [1] == 'P' && filename [2] == 'S' &&
				UTIL_ISDIGIT (filename [3]) && UTIL_ISDIGIT (filename [4]) && UTIL_ISDIGIT (filename [5]) && 
						UTIL_ISDIGIT (filename [6]) && UTIL_ISDIGIT (filename [7]) &&
				filename [8] == '.' && filename [9] == 'T' && filename [10] == 'X' && filename [11] == 'T') ? TRUE : FALSE;
}

static boolean cmd_call__file_list (const cmd_arg_t* const arg)
{
		fat_dirent_t dirent;
		fat_error_t result;

		UNUSED (arg);

		if (fat_dirent_init (&dirent) != FAT_ERR_NONE)
				return FALSE;

		/*@-infloopsuncon@*/ while ((result = fat_dirent_next (&dirent)) == FAT_ERR_NONE) /*@=infloopsuncon@*/
		{
				if (valid_file (dirent.name) == TRUE)
				{
						DPRINTF (("%-8lu \"%s\"\n", dirent.size, dirent.name)); /*XXX*/
				}
		}

		return result == FAT_ERR_ENDOFDIRECTORY ? TRUE : FALSE;
}

static boolean cmd_call__file_remove (const cmd_arg_t* const arg)
{
		if (arg == NULL || arg [0] == NULL)
				return FALSE;

		if (valid_file (arg [0]) == FALSE)
				return FALSE;

		return fat_unlink (arg [0]) == FAT_ERR_NONE ? TRUE : FALSE;
}

static boolean cmd_call__file_clear (const cmd_arg_t* const arg)
{
		fat_dirent_t dirent;
		fat_error_t result;

		UNUSED (arg);

		if (fat_dirent_init (&dirent) != FAT_ERR_NONE)
				return FALSE;

		/*@-infloopsuncon@*/ while ((result = fat_dirent_next (&dirent)) == FAT_ERR_NONE) /*@=infloopsuncon@*/
		{
				if (valid_file (dirent.name) == TRUE)
				{
						if (fat_unlink (dirent.name) != FAT_ERR_NONE)
								return FALSE;
				}
		}

		return result == FAT_ERR_ENDOFDIRECTORY ? TRUE : FALSE;
}

static boolean cmd_call__file_display (const cmd_arg_t* const arg)
{
		if (arg == NULL || arg [0] == NULL)
				return FALSE;

		if (valid_file (arg [0]) == FALSE)
				return FALSE;

		return cmd_util_display (arg [0]);
}

static boolean cmd_call__file_transfer (const cmd_arg_t* const arg)
{
		if (arg == NULL || arg [0] == NULL)
				return FALSE;

		if (valid_file (arg [0]) == FALSE)
				return FALSE;

		return cmd_util_transfer (arg [0]);
}

/* ----------------------------------------------------------------------------------------------------*/

static boolean cmd_call__position_get (const cmd_arg_t* const arg)
{
		UNUSED (arg);

		DPRINTF (("%s\n", gps_capture_get ())); /*XXX*/

		return TRUE;
}

static boolean cmd_call__position_save (const cmd_arg_t* const arg)
{
		return gps_capture_save (arg [0]);
}

static boolean cmd_call__position_clear (const cmd_arg_t* const arg)
{
		UNUSED (arg);

		return gps_capture_clear ();
}

static boolean cmd_call__position_display (const cmd_arg_t* const arg)
{
		boolean result;

		UNUSED (arg);

		if (gps_capture_suspend () == FALSE)
				return FALSE;

		result = cmd_util_display (gps_capture_filename ());

		if (gps_capture_resume () == FALSE)
				return FALSE;

		return result;
}

static boolean cmd_call__position_transfer (const cmd_arg_t* const arg)
{
		boolean result;

		UNUSED (arg);

		if (gps_capture_suspend () == FALSE)
				return FALSE;

		result = cmd_util_transfer (gps_capture_filename ());

		if (gps_capture_resume () == FALSE)
				return FALSE;

		return result;
}

/* ----------------------------------------------------------------------------------------------------*/

static const cmd_item_t cmd_item_list [] =
{
		{ { "version", NULL }, { CMD_ARG_NONE, CMD_ARG_NONE }, NULL,
			"Version information about the device hardware and firmware", cmd_call__version },
		{ { "diagnostics", NULL }, { CMD_ARG_NONE, CMD_ARG_NONE }, NULL,
			"Diagnostic information about current operation of the device", cmd_call__diagnostics },

		{ { "reset", NULL }, { CMD_ARG_NONE, CMD_ARG_NONE }, NULL,
			"Reset the device to reload firmware and restart operation", cmd_call__reset },

#ifdef DEBUG
		{ { "debug", "test" }, { CMD_ARG_NONE, CMD_ARG_NONE }, NULL,
			"Debug: execute the built in test cases, and reset the device", cmd_call__stub },
		{ { "debug", "health" }, { CMD_ARG_NONE, CMD_ARG_NONE }, NULL,
			"Debug: perform a brief health check of the device", cmd_call__stub },
		{ { "debug", "display" }, { CMD_ARG_NONE, CMD_ARG_NONE }, NULL,
			"Debug: toggle display of executing trace information", cmd_call__stub },
		{ { "debug", "read" }, { CMD_ARG_HEX, CMD_ARG_NUM|CMD_ARG_OPT }, "locn [nbytes]",
			"Debug: read byte(s) from specified memory location, default to 64 bytes (hexadecimal location, and number of bytes)", cmd_call__stub },
		{ { "debug", "write" }, { CMD_ARG_HEX, CMD_ARG_HEX }, "locn byte",
			"Debug: write byte to specified memory location (hexademical location and single byte)", cmd_call__stub },
#endif

		{ { "date", "get" }, { CMD_ARG_NONE, CMD_ARG_NONE }, NULL,
			"Get the current date/time from the clock and display", cmd_call__date_get },
		{ { "date", "set" }, { CMD_ARG_ANY, CMD_ARG_NONE }, "yyyymmddHHMMSS",
			"Set the current date/time for the clock", cmd_call__date_set },

		{ { "config", "get" }, { CMD_ARG_ANY, CMD_ARG_NONE }, "name",
			"Config: get specific config parameter with current value and display", cmd_call__config_get },
		{ { "config", "set" }, { CMD_ARG_ANY, CMD_ARG_ANY }, "name value",
			"Config: set specific config parameter to a provided value (and write to file)", cmd_call__config_set },
		{ { "config", "clear" }, { CMD_ARG_NONE, CMD_ARG_NONE }, NULL,
			"Config: clear config parameters from file (so that defaults will be written, must reset)", cmd_call__config_clear },
		{ { "config", "display" }, { CMD_ARG_NONE, CMD_ARG_NONE }, NULL,
			"Config: display all configuration parameters with current and default values", cmd_call__config_display },
		{ { "config", "transfer" }, { CMD_ARG_NONE, CMD_ARG_NONE }, NULL,
			"Config: transfer (using YModem-G) the configuration parameters", cmd_call__config_transfer },
			
		{ { "file", "list" }, { CMD_ARG_NONE, CMD_ARG_NONE }, NULL,
			"File: list all files on the file system", cmd_call__file_list },
		{ { "file", "remove" }, { CMD_ARG_ANY, CMD_ARG_NONE }, "filename.ext",
			"File: remove a specific logfile from the file system", cmd_call__file_remove },
		{ { "file", "clear" }, { CMD_ARG_NONE, CMD_ARG_NONE }, NULL,
			"File: clear all logfiles from the file system", cmd_call__file_clear },
		{ { "file", "display" }, { CMD_ARG_ANY, CMD_ARG_NONE }, "filename.ext",
			"File: display a specific logfile from the file system", cmd_call__file_display },
		{ { "file", "transfer" }, { CMD_ARG_ANY, CMD_ARG_NONE }, "filename.ext",
			"File: transfer (using YModem-G) specific file from the file system", cmd_call__file_transfer },

		{ { "position", "get" }, { CMD_ARG_NONE, CMD_ARG_NONE }, NULL,
			"Position: get the current position and display", cmd_call__position_get },
		{ { "position", "save" }, { CMD_ARG_ANY|CMD_ARG_OPT, CMD_ARG_NONE }, "[text comment]",
			"Position: save a stored current position with provided tag (and write to file)", cmd_call__position_save },
		{ { "position", "clear" }, { CMD_ARG_NONE, CMD_ARG_NONE }, NULL,
			"Position: clear all stored positions (and write to file)", cmd_call__position_clear },
		{ { "position", "display" }, { CMD_ARG_NONE, CMD_ARG_NONE }, NULL,
			"Position: display all stored positions (by read from file)", cmd_call__position_display },
		{ { "position", "transfer" }, { CMD_ARG_NONE, CMD_ARG_NONE }, NULL,
			"Position: transfer (using YModem-G) all stored positions", cmd_call__position_transfer }
};

static const cmd_config_t cmd_config =
{
		CMD_TEXT_PROMPT,
		CMD_TEXT_HELP,
		cmd_item_list,
		(unsigned char) (sizeof (cmd_item_list) / sizeof (cmd_item_list [0]))
};

/* ----------------------------------------------------------------------------------------------------*/

/*@-compmempass@*/
const cmd_config_t* main_cfg_command (void)
{
		return &cmd_config;
}
/*@=compmempass@*/

/* ----------------------------------------------------------------------------------------------------*/

#ifdef TEST_ENABLED

#define CMD_SKEY(c,n) (((c)->key [n] == NULL) ? '\0' : UTIL_TOUPPER ((c)->key [n][0]))

test_result_t main_command_test (void)
{
		int i, j, n = (int) (sizeof (cmd_item_list) / sizeof (cmd_item_list [0]));

		DPRINTF (("main_command_test: %d commands\n", n));

		test_assert (main_cfg_command () != NULL);

		test_assert (cmd_config.text_prompt != NULL && cmd_config.text_help != NULL);
		test_assert (cmd_config.item_list != NULL && cmd_config.item_size > (unsigned char) 0);

		for (i = 0; i < n; i++)
		{
				const cmd_item_t * const cmd = &cmd_item_list [i];
				char cmdshort [2] = { CMD_SKEY (cmd, 0), CMD_SKEY (cmd, 1) };

				test_assert (cmd->key [0] != NULL);
				test_assert (!(cmd->arg_flags [0] == CMD_ARG_NONE && cmd->arg_flags [1] != CMD_ARG_NONE));
				test_assert ((cmd->arg_flags [0] != CMD_ARG_NONE) ? (cmd->args != NULL) : (cmd->args == NULL));
				test_assert (cmd->help != NULL);
				test_assert (cmd->call != NULL);

				for (j = (i + 1); j < n; j++)
				{
						const cmd_item_t * const cmd1 = &cmd_item_list [j];
						char cmd1short [2] = { CMD_SKEY (cmd1, 0), CMD_SKEY (cmd1, 1) };

						test_assert (!(cmdshort [0] == cmd1short [0] && cmdshort [1] == cmd1short [1]));
				}
		}

		return TEST_RESULT_OKAY;
}

#endif

/* ----------------------------------------------------------------------------------------------------*/
