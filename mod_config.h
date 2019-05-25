
/* ----------------------------------------------------------------------------------------------------*/
/*
	mod_config.h: gps logger module for configuration handling.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#ifndef MOD_CONFIG_H_
#define MOD_CONFIG_H_

/* ----------------------------------------------------------------------------------------------------*/

typedef enum
{
		t_integer,
		t_boolean,
		t_string
}
cfg_type_t;

typedef unsigned char cfg_flag_t;

#define CFG_FLAG_FORMAT_NONE			  ((cfg_flag_t) 0x00)
#define CFG_FLAG_FORMAT_TIMEPERIOD		  ((cfg_flag_t) 0x01)
#define CFG_FLAG_FORMAT_BASE2BYTES		  ((cfg_flag_t) 0x02)
#define CFG_FLAG_FORMAT_CSV				  ((cfg_flag_t) 0x04)

#define CFG_STRING_SIZE		32

typedef union
{
		boolean (*c_boolean) (const boolean);
		boolean (*c_integer) (const signed long);
		boolean (*c_string) (const char * const);
}
cfg_call_t;

typedef boolean cfg_data_boolean_t;
typedef signed long cfg_data_integer_t;
typedef char cfg_data_string_t [CFG_STRING_SIZE];

typedef union
{
		cfg_data_boolean_t * const d_boolean;
		cfg_data_integer_t * const d_integer;
		cfg_data_string_t * const d_string;
}
cfg_data_t;

typedef struct
{
		/*@notnull@*/ /*@observer@*/ const char * const name;
		/*@null@*/ /*@observer@*/ const char * const description_name;
		/*@null@*/ /*@observer@*/ const char * const description_type;
		const cfg_type_t type;
		const cfg_flag_t flag;
		const cfg_call_t call;
		const cfg_data_t data;
}
cfg_item_t;

typedef struct
{
		/*@notnull@*/ /*@observer@*/ const char * const filename;
		/*@notnull@*/ /*@observer@*/ const char * const default_header;
		/*@notnull@*/ /*@observer@*/ const char * const default_footer;
		/*@notnull@*/ /*@shared@*/ const cfg_item_t * const item_list;
		const unsigned char item_size;
}
cfg_config_t;

/* ----------------------------------------------------------------------------------------------------*/

boolean cfg_init (/*@notnull@*/ const cfg_config_t * const config, const int argc, /*@notnull@*/ const char ** const argv);

/*@notnull@*/ /*@observer@*/ const char * cfg_filename (/*@notnull@*/ const cfg_config_t * const config);

/*@null@*/ /*@shared@*/ const cfg_item_t * cfg_item_get (/*@notnull@*/ const cfg_config_t * const config, /*@notnull@*/ const char * const name);
boolean cfg_item_set (/*@notnull@*/ const cfg_config_t * const config, /*@notull@*/ const char * const name, /*@notull@*/ const char * const data);

#ifdef TEST_ENABLED
test_result_t cfg_test (void);
#endif

/* ----------------------------------------------------------------------------------------------------*/

#endif /*MOD_CONFIG_H_*/

