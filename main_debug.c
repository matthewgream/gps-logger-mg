
/* ----------------------------------------------------------------------------------------------------*/
/*
	main_debug.c: gps logger main debug mechanisms.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#include "common.h"
#include "mod_util.h"
#include "mod_lpc21XXhal.h"
#include "mod_filespisd.h"
#include "mod_filefatfs.h"
#include "mod_filecache.h"
#include "mod_config.h"
#include "mod_command.h"
#include "mod_packlzari.h"
#include "mod_xferymodem.h"
#include "mod_gps.h"

#include "main_config.h"
#include "main_command.h"
#include "main_debug.h"

/* ----------------------------------------------------------------------------------------------------*/

#ifdef TEST_ENABLED
static test_result_t main_debug_test (void);
#endif

/* ----------------------------------------------------------------------------------------------------*/

#ifdef DEBUG

static char debug_port = (char) -1;

boolean debug_trace = TRUE;

void debug_init (void)
{
		lpc21XX_uart0_init ((unsigned int) 9600, FALSE);
		debug_port = (char) 0;
}

void debug_term (void)
{
		/* leave the port hanging open ... */
		/*debug_port = (char) -1;
		lpc21XX_uart0_term ();*/
}

boolean debug_mode (const char * const mode)
{
		assert (mode != NULL);

		if (util_strcmpi (mode, "diag"))
		{
				DPRINTF (("diagnostics mode: running internal test suite\n"));
				test_runner (lpc21XX_test);
				test_runner (util_test);
				test_runner (card_test);
				test_runner (fat_test);
				test_runner (cfg_test);
				test_runner (command_test);
				test_runner (cachefile_test);
				test_runner (packlzari_test);
				test_runner (ymodem_test);
				test_runner (gps_input_test);
				test_runner (gps_format_test);
				test_runner (gps_output_test);
				test_runner (gps_process_test);
				test_runner (gps_capture_test);
				test_runner (gps_test);
				test_runner (main_config_test);
				test_runner (main_command_test);
				test_runner (main_debug_test);
				DPRINTF (("diagnostics mode: tests completed, no errors\n\n"));
				lpc21XX_down ();
				/*@notreached@*/
		}
		else if (util_strcmpi (mode, "trace"))
		{
				debug_trace = TRUE;
		}
		else if (util_strcmpi (mode, "none"))
		{
				debug_trace = FALSE;
		}
		else
		{
				return FALSE;
		}

		return TRUE;
}

/*@-namechecks@*/
void __assert (const char* const filename, const unsigned int fileline, const char* const expression)
{
		HALT (("assert[%s:%d] - %s\n", filename, fileline, expression));
}
/*@=namechecks@*/

/*@-namechecks@*/
int __putchar (int ch)
{
/*@-elseifcomplete@*/
		if (debug_port == (char) 0) {
				if (ch == (int) '\n')
						lpc21XX_uart0_putc ('\r');
				lpc21XX_uart0_putc ((unsigned char) ch);
		} else if (debug_port == (char) 1) {
				if (ch == (int) '\n')
						lpc21XX_uart1_putc ('\r');
				lpc21XX_uart1_putc ((unsigned char) ch);
		}
/*@=elseifcomplete@*/
		return ch;
}
/*@=namechecks@*/

#else

/*@-namechecks@*/
int __putchar (int ch)
{
		UNUSED (ch);
		return 1;
}
/*@=namechecks@*/

#endif

/* ----------------------------------------------------------------------------------------------------*/

#ifdef TEST_ENABLED

static test_result_t main_debug_test (void)
{
		return TEST_RESULT_OKAY;
}

#endif

/* ----------------------------------------------------------------------------------------------------*/

