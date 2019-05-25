
/* ----------------------------------------------------------------------------------------------------*/
/*
	mod_command.h: gps logger module for command handling.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#ifndef MOD_COMMAND_H_
#define MOD_COMMAND_H_

/* ----------------------------------------------------------------------------------------------------*/

typedef /*@null@*/ const char * const cmd_key_t;

typedef /*@null@*/ const char * const cmd_arg_t;

typedef unsigned char cmd_arg_flags_t;
#define CMD_ARG_NONE ((cmd_arg_flags_t) 0x00)
#define CMD_ARG_ANY ((cmd_arg_flags_t) 0x02)
#define CMD_ARG_NUM ((cmd_arg_flags_t) 0x04)
#define CMD_ARG_HEX ((cmd_arg_flags_t) 0x08)
#define CMD_ARG_OPT ((cmd_arg_flags_t) 0x10)

typedef boolean (*cmd_item_call_t) (const cmd_arg_t* const arg);

typedef struct
{
		/*@observer@*/ const cmd_key_t key [2];
		/*@observer@*/ const cmd_arg_flags_t arg_flags [2];
		/*@null@*/ /*@observer@*/ const char * const args;
		/*@null@*/ /*@observer@*/ const char * const help;
		/*@notnull@*/ const cmd_item_call_t call;
}
cmd_item_t;

typedef struct
{
		/*@notnull@*/ /*@observer@*/ const char * const text_prompt;
		/*@notnull@*/ /*@observer@*/ const char * const text_help;
		/*@notnull@*/ /*@shared@*/ const cmd_item_t * const item_list;
		const unsigned char item_size;
}
cmd_config_t;

/* ----------------------------------------------------------------------------------------------------*/

boolean cmd_init (/*@notnull@*/ const cmd_config_t * const config);

#ifdef TEST_ENABLED
test_result_t command_test (void);
#endif

/* ----------------------------------------------------------------------------------------------------*/

#endif /*MOD_COMMAND_H_*/

