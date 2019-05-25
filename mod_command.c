/* ----------------------------------------------------------------------------------------------------*/
/*
	mod_command.c: gps logger module for command interface.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#include "common.h"
#include "mod_util.h"
#include "mod_command.h"

/* ----------------------------------------------------------------------------------------------------*/

/* '?' command */

/* ----------------------------------------------------------------------------------------------------*/

/* 'help' command */

/* ----------------------------------------------------------------------------------------------------*/

boolean cmd_init (const cmd_config_t * const config)
{
		assert (config != NULL);
		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

#ifdef TEST_ENABLED

test_result_t command_test (void)
{
		return TEST_RESULT_OKAY;
}

#endif

/* ----------------------------------------------------------------------------------------------------*/

#if 0

void cmd_handler (const cmd_definition_t* const defin, const cmd_arguments_t* cond args, io_handler_t io)
typedef unsigned char cmd_arg_t;
typedef cmd_arg_t cmd_args_t [2]; // max two args
typedef unsigned char* cmd_word_t;
typedef cmd_word_t cmd_words_t [2] // max two words

typedef struct {
		cmd_words_t words; // "config set"
		cmd_args_t args;   // ALNUM ALNUM|OPT
		cmd_handler_t handler; //
		const char * const description;
} cmd_definition_t;
cmd_help

cmd_trigger ?
for 'G'/'C' (auto download and delete)

the self tests should warn if multiple commands map to each other in short or long forms

config? io_handler? for ymodem, should pass in some read/write handler pointers - take care of them
locally with some glue functions

config file:
cmdinterface: enabled?
new waypoint module

1. need new position module
2. need updated interfaces to existing modules
3. may need new file list function in dos module
4. need ymodem module, and io glue for it

/* ----------------------------------------------------------------------------------------------------*/

boolean cmd_init (const cmd_config_t * const config, ... serial handle ...)
{
}

boolean cmd_handle (/*@notnull@*/ const unsigned char * const buffer, const unsigned int length);
{
		// recv and echo char
		// we just store it, don't do any processing until EOL, support BS/etc though

		// at EOL, we need to parse ...
		// ... each command is a set of keywords, and set of arguments (of certain types)
		// check if we got the keywords, and each argument (check that it is of hte type)
		// call the handler with the arguments

gps_logger_mg v0.94b-dev (hw: sparkfun-2.4/lpc21xx/em406): command line interface
glc: version
....
glc: reset



		// commands:
		//	- ?
"List all available commands in the command interface"
		//	- help
"Provide detailed help for use of the command interface"

------------------------------------------------------

The command line interface must be enabled in the configuration, then after reset it
will become active. If it is disabled in the configuration, then reset must occur to
effect change. Note that any configuration change only has effect after reset.

The interface provides a prompt labelled "glc: " (for 'gps logger command') at which
commands (whether by user or another program) can be entered to control the device.
Clear, delete and backspace characters are supported. All inputs are case insensitive.

For help, use '?' for a command list, and 'help' for detailed command description.

All commands may be used in a short or long form. In the long form, either the all,
or any number of initial letters, in each word can be used as long as spacing is
retained. In the short form, the initial letter of each word can be concatenated.
The arguments have limited type checking and wildcard support (where noted). The
allowable wildcards are '?' or '.' (match any single character) and '*' (match any
sequence of characters). All commands must be terminated by a carraige return.

For example, some commands with their long and short form equivalents are:
- 'config clear', 'con cl', 'config c' (long forms); 'cc' (short form)
- 'file transfer gps*.dat', 'f t gps*.dat' (long forms); 'ft gps*.dat' (short form)
- 'reset', 'res', 'r' (long forms); 'r' (short form)

Please refer to the manual for more detail about individual commands and features.

------------------------------------------------------

		//	- file clear
"Clear all logfiles from the file system"
		//	- file remove X
"Remove a specific, or pattern matching, logfiles from the file system"
		//	- file display X
"Display specific, or pattern matching logfiles from the file system"
		//	- file transfer X
"Transfer (using YModem-G) specific, or pattern matching, files from the file system"
		//	- file list
"List all files on the file system"
		//	- file number
"Number of files on the file system"
		//	- date get
"Get the current date/time from the clock and display"
		//	- date set x
"Set the current date/time for the clock"
		//	- position clear
"Clear all stored positions (and write to file)"
		//	- position get
"Get the current position and display"
		//	- position save [x]
"Save a stored current position with provided tag (and write to file)"
		//	- position display
"Display all stored positions (by read from file)"
		//	- position number
"Number of stored positions"
		//	- position transfer
"Transfer (using YModem-G) all stored positions"
		//	- config clear
"Clear all configuration parameters back to default values (and write to file)"
		//	- config get x
"Get specific configuration parameter with current value and display"
		//	- config set x [y]
"Set specific configuration parameter to a provided value or to its default value (and write to file)"
		//	- config display
"Display all configuration parameters with current and default values"
		//	- config transfer
"Transfer (using YModem-G) the configuration parameters"
		//	- version
"Provide detailed version information about the device hardware and firmware"
		//	- reset
"Execute a soft reset of the device, to effect firmware restart/reinitialisation"
		//	- diagnostics
"Provide diagnostic information about current operation of the device"
		//	- debug test
"Debug: execute the built in test cases, and reset the device"
		//	- debug health
"Debug: perform a brief health check of the device"
		//	- debug display
"Debug: toggle display of executing trace information"
		//	- debug read X [C]
"Debug: read byte(s) from specified memory location, default to 64 bytes"
		//	- debug write X Y
"Debug: write byte to specified memory location"

stored position:
LAT,LON,ALT,DAT,DESCR

		// command data:
		//	{ "file", "print", NULL }, { CMD_ARG_ALPHANUMERIC|CMD_FLG_OPTIONAL, NULL },
		//		...
}

#endif

/* ----------------------------------------------------------------------------------------------------*/

