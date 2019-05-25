
/* ----------------------------------------------------------------------------------------------------*/
/*
	main_command.h: gps logger main command data.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#ifndef MAIN_COMMAND_H_
#define MAIN_COMMAND_H_

/* ----------------------------------------------------------------------------------------------------*/

/*@observer@*/ /*@notnull@*/ const cmd_config_t * main_cfg_command (void);

#ifdef TEST_ENABLED
test_result_t main_command_test (void);
#endif

/* ----------------------------------------------------------------------------------------------------*/

#endif /*MAIN_COMMAND_H_*/

