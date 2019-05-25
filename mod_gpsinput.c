
/* ----------------------------------------------------------------------------------------------------*/
/*
	mod_gpsinput.c: gps logger module for GPS processing.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#include "common.h"
#include "mod_lpc21XXhal.h"
#include "mod_gpsinput.h"

/* ----------------------------------------------------------------------------------------------------*/
/* EM-406 ... */
/* Lassen iQ */
/* ----------------------------------------------------------------------------------------------------*/

#if 0

#define GPS_TSIP_BUFFER_SIZE 32

#define GPS_TSIP_CHAR_DLE 0x10
#define GPS_TSIP_CHAR_ETX 0x03

#define GPS_TSIP_COMMAND_SOFTRESET			  0x25
#define GPS_TSIP_COMMAND_CONFIGOUTPUTTSIP	  0x35
#define GPS_TSIP_COMMAND_CONFIGSENSITIVITY	  0x69
#define GPS_TSIP_COMMAND_CONFIGOUTPUTNMEA	  0x7A
#define GPS_TSIP_COMMAND_CONFIGPORT			  0xBC

static void gps_tsip_send (const unsigned char id, const unsigned int size, const unsigned char * const data)
{
		lpc21XX_uart1_putc (GPS_TSIP_CHAR_DLE);
		lpc21XX_uart1_putc (id);

		if (data != NULL && size > 0)
		{
				unsigned int offs = 0;

				while (offs < size)
				{
						unsigned char data_byte = data [offs++];

						if (data_byte == GPS_TSIP_CHAR_DLE)
						{
								lpc21XX_uart1_putc (GPS_TSIP_CHAR_DLE);
						}

						lpc21XX_uart1_putc (data_byte);
				}
		}

		lpc21XX_uart1_putc (GPS_TSIP_CHAR_DLE);
		lpc21XX_uart1_putc (GPS_TSIP_CHAR_ETX);
}

#endif

/* ----------------------------------------------------------------------------------------------------*/

unsigned int gps_input_read (/*@out@*/ unsigned char * data, const unsigned int size)
{
		unsigned int count = 0;

		assert (data != NULL);
		assert (size > 0);

		/* leds: v1.0 toggle stat0, v2.4: toggle red */
		if ((data [count++] = lpc21XX_uart1_getc ()) != (unsigned char) '\n') {
				lpc21XX_led_enable ((unsigned char) 0);
				count = lpc21XX_uart1_gets (&data [count], (size - 1) - count);
				lpc21XX_led_disable ((unsigned char) 0);
		}
		data [count] = (unsigned char) '\0';

		return count + 1;
}

/* ----------------------------------------------------------------------------------------------------*/

boolean gps_input_init (const gps_config_input_t * const config)
{
#if 0
		unsigned char tsip_buffer [GPS_TSIP_BUFFER_SIZE];
#endif
		assert (config != NULL);

		UNUSED (config);

		lpc21XX_gpshw_enable ();

		lpc21XX_uart1_init ((unsigned int) 4800, TRUE);
		lpc21XX_uart1_set_fifo_level (LPC21XX_UART_FIFO_LEVEL_MAX);

#if 0
		tsip_buffer [0] = 0x01;
		gps_tsip_send (GPS_TSIP_COMMAND_CONFIGSENSITIVITY, 1, tsip_buffer);
		gps_tsip_send (GPS_TSIP_COMMAND_SOFTRESET, 0, NULL);

		/*
		// configure enhanced sensitivity
		// send 0x69 (enhanced sensitivity mode) with no content
		// read 0x89 with byte 0 = ?, byte 1 = ?
		// if byte 0 = 0
		//		  send 0x69 with byte 0 = 1
		//		  read 0x89 with byte 0 = 0, byte 1 = 1
		//		  send 0x25 (soft reset)
		//		  read 0x45 (software version info)
		//		  send 0x69
		//		  read 0x89 with byte 0 = 1, byte 1 = 1

		// configure NMEA reporting output
		// send 0x7A with bits (see page 153) to enable GGA and ZDA (if RTC enabled)
		// (make this configurable): GGA, GLL, VTG, GSV, GSA, ZDA, RMC
		// fix interval in seconds: 1 (perhaps higher)

		// configure NMEA port output
		// send 0xBC (see page 164) to set port details / baud rates / TSIP / NMEA / etc

		// turn off any output on port 1
		// send 0x35, byte 0 = 0, byte 1 = 0 (check docs, page 121)
		// turn off pps
		*/
#endif

		DPRINTF (("gps_port = 1 (NMEA)\n"));

		return TRUE;
}

boolean gps_input_term (void)
{
		lpc21XX_uart1_term ();

		lpc21XX_gpshw_disable ();

		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

#ifdef TEST_ENABLED

test_result_t gps_input_test (void) /* XXX */
{
		return TEST_RESULT_OKAY;
}

#endif

/* ----------------------------------------------------------------------------------------------------*/

