
/* ----------------------------------------------------------------------------------------------------*/
/*
	main_config.h: gps logger main configuration data.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#ifndef MAIN_CONFIG_H_
#define MAIN_CONFIG_H_

/* ----------------------------------------------------------------------------------------------------*/

/*@observer@*/ /*@notnull@*/ const cfg_config_t * main_cfg_config (void);

#ifdef TEST_ENABLED
test_result_t main_config_test (void);
#endif

/* ----------------------------------------------------------------------------------------------------*/

#endif /*MAIN_CONFIG_H_*/

