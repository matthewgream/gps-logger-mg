
/* ----------------------------------------------------------------------------------------------------*/
/*
	mod_gpsprocess.c: gps logger module for GPS processing.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#include "common.h"
#include "mod_lpc21XXhal.h"
#include "mod_gpsprocess.h"

/* ----------------------------------------------------------------------------------------------------*/

static gps_config_process_t gps_process_config;

/* ----------------------------------------------------------------------------------------------------*/

boolean gps_process_handle (const gps_nmea_sentence_t * const content)
{
		assert (content != NULL);

		if (gps_process_config.update_rtc)
		{
				if (content->type == gps_nmea_sentence_type_zda) /* drive from ZDA */
				{
						lpc21XX_rtc_t c;
						c.e [LPC21XX_RTC_ELEM_SS] = (unsigned char) (((content->stru.zda.utc [4] - '0') * (char) 10) + (content->stru.zda.utc [5] - '0'));
						c.e [LPC21XX_RTC_ELEM_MM] = (unsigned char) (((content->stru.zda.utc [2] - '0') * (char) 10) + (content->stru.zda.utc [3] - '0'));
						c.e [LPC21XX_RTC_ELEM_HH] = (unsigned char) (((content->stru.zda.utc [0] - '0') * (char) 10) + (content->stru.zda.utc [1] - '0'));
						c.e [LPC21XX_RTC_ELEM_DD] = (unsigned char) (((content->stru.zda.dd [0] - '0') * (char) 10) + (content->stru.zda.dd [1] - '0'));
						c.e [LPC21XX_RTC_ELEM_MO] = (unsigned char) (((content->stru.zda.mm [0] - '0') * (char) 10) + (content->stru.zda.mm [1] - '0'));
						c.e [LPC21XX_RTC_ELEM_YY] = (unsigned char) ((((unsigned int) (content->stru.zda.yyyy [0] - '0') * 1000) + 
																	((unsigned int) (content->stru.zda.yyyy [1] - '0') * 100) +
																	((unsigned int) (content->stru.zda.yyyy [2] - '0') * 10) +
																	(unsigned int) (content->stru.zda.yyyy [3] - '0')) - 1980);
						lpc21XX_rtc_set (&c);
				}
				else if (content->type == gps_nmea_sentence_type_rmc) /* drive from RMC */
				{
						lpc21XX_rtc_t c;
						c.e [LPC21XX_RTC_ELEM_SS] = (unsigned char) (((content->stru.rmc.utc [4] - '0') * (char) 10) + (content->stru.rmc.utc [5] - '0'));
						c.e [LPC21XX_RTC_ELEM_MM] = (unsigned char) (((content->stru.rmc.utc [2] - '0') * (char) 10) + (content->stru.rmc.utc [3] - '0'));
						c.e [LPC21XX_RTC_ELEM_HH] = (unsigned char) (((content->stru.rmc.utc [0] - '0') * (char) 10) + (content->stru.rmc.utc [1] - '0'));
						c.e [LPC21XX_RTC_ELEM_DD] = (unsigned char) (((content->stru.rmc.dd [0] - '0') * (char) 10) + (content->stru.rmc.dd [1] - '0'));
						c.e [LPC21XX_RTC_ELEM_MO] = (unsigned char) (((content->stru.rmc.mm [0] - '0') * (char) 10) + (content->stru.rmc.mm [1] - '0'));
						c.e [LPC21XX_RTC_ELEM_YY] = (unsigned char) ((((unsigned int) ((content->stru.rmc.yy [0] > '7' ? '1' : '2') - '0') * 1000) + 
																	((unsigned int) ((content->stru.rmc.yy [0] > '7' ? '9' : '0') - '0') * 100) +
																	((unsigned int) (content->stru.rmc.yy [0] - '0') * 10) +
																	(unsigned int) (content->stru.rmc.yy [1] - '0')) - 1980);
						lpc21XX_rtc_set (&c);
				}
		}

		return TRUE;
}

boolean gps_process_init (const gps_config_process_t * const config)
{
		assert (config != NULL);

		gps_process_config = *config;

		if (gps_process_config.update_rtc)
		{
				lpc21XX_rtc_enable ();
		}

		return TRUE;
}

boolean gps_process_term (void)
{
		if (gps_process_config.update_rtc)
		{
				lpc21XX_rtc_disable ();
		}

		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

#ifdef TEST_ENABLED

test_result_t gps_process_test (void) /* XXX */
{
		return TEST_RESULT_OKAY;
}

#endif

/* ----------------------------------------------------------------------------------------------------*/

