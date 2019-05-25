
/* ----------------------------------------------------------------------------------------------------*/
/*
	mod_config.c: gps logger module for configuration handling.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#include "common.h"
#include "mod_util.h"
#include "mod_filefatfs.h"
#include "mod_config.h"

/* ----------------------------------------------------------------------------------------------------*/

static boolean cfg_config_write = TRUE;

/* ----------------------------------------------------------------------------------------------------*/

const char * cfg_filename (const cfg_config_t * const config)
{
		assert (config != NULL);

		return config->filename;
}

/* ----------------------------------------------------------------------------------------------------*/

/*@observer@*/ static const char* cfg_type_string (const cfg_type_t type)
{
		if (type == t_integer) return "integer";
		else if (type == t_boolean) return "boolean";
		else if (type == t_string) return "string";
		else return "undefined";
}

/* ----------------------------------------------------------------------------------------------------*/

static boolean cfg_item_decode_detokenise (/*@notnull@*/ char * const string, /*@notnull@*/ /*@out@*/ char ** name_r, /*@notnull@*/ /*@out@*/ char ** data_r)
{
		char * name, * name_end;
		char * data, * data_end;
		char * tmp;

		assert (string != NULL);
		assert (name_r != NULL);
		assert (data_r != NULL);

		if (*string == '\0')
		{
				return FALSE;
		}

		name = string;
		while (*name != '\0' && *name != '#' && UTIL_ISSPACE (*name))
		{
				name++;
		}
		if (*name == '\0' || *name == '#')
		{
				return FALSE;
		}

		name_end = name;
		while (*name_end != '\0' && *name_end != '=')
		{
				name_end++;
		}
		if (*name_end == '\0')
		{
				return FALSE;
		}
		tmp = name_end;
		while (name_end != name && (UTIL_ISSPACE (*name_end) || *name_end == '='))
		{
				name_end--;
		}
		if (name_end == name)
		{
				return FALSE;
		}
		name_end++;

		data = &tmp [1];
		while (*data != '\0' && *data != '#' && UTIL_ISSPACE (*data))
		{
				data++;
		}
		if (*data == '\0' || *data == '#')
		{
				return FALSE;
		}

		data_end = data;
		while (*data_end != '\0' && *data_end != '#')
		{
				data_end++;
		}
		while (data_end != data && (UTIL_ISSPACE (*data_end) || *data_end == '#'))
		{
				data_end--;
		}
		if (data_end == data)
		{
				return FALSE;
		}
		if (*data_end != '\0')
		{
				data_end++;
		}

		*name_end = '\0';
		*data_end = '\0';

		*name_r = name;
		*data_r = data;

		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

static boolean cfg_item_decode_boolean (/*@notnull@*/ const cfg_item_t * const item, /*@notnull@*/ const char * data)
{
		boolean d_boolean;

		assert (item != NULL);
		assert (data != NULL);

		if (UTIL_TOUPPER (*data) == 'T' || UTIL_TOUPPER (*data) == 'Y')
				d_boolean = TRUE;
		else if (UTIL_TOUPPER (*data) == 'F' || UTIL_TOUPPER (*data) == 'N')
				d_boolean = FALSE;
		else
				return FALSE;

		if (item->call.c_boolean != NULL && (*item->call.c_boolean) (d_boolean) == FALSE)
				return FALSE;

		DPRINTF (("cfg_item_decode_boolean: got '%s'\n", d_boolean ? "true" : "false"));
		*(item->data.d_boolean) = d_boolean;

		return TRUE;
}

static boolean cfg_item_decode_integer (/*@notnull@*/ const cfg_item_t * const item, /*@notnull@*/ const char * data)
{
		signed long d_integer = 0;
		boolean d_negative = FALSE;

		assert (item != NULL);
		assert (data != NULL);

		while (*data != '\0')
		{
				if (*data == '-')
						d_negative = TRUE;
				else if (*data == '+')
						d_negative = FALSE;
				else if (*data >= '0' && *data <= '9')
						d_integer = (d_integer * 10) + (signed long)(*data - '0');
				else if (UTIL_TOUPPER (*data) == 'D' && (item->flag & CFG_FLAG_FORMAT_TIMEPERIOD) != CFG_FLAG_FORMAT_NONE)
						d_integer *= (60*60*24);
				else if (UTIL_TOUPPER (*data) == 'H' && (item->flag & CFG_FLAG_FORMAT_TIMEPERIOD) != CFG_FLAG_FORMAT_NONE)
						d_integer *= (60*60);
				else if (UTIL_TOUPPER (*data) == 'M' && (item->flag & CFG_FLAG_FORMAT_TIMEPERIOD) != CFG_FLAG_FORMAT_NONE)
						d_integer *= (60);
				else if (UTIL_TOUPPER (*data) == 'S' && (item->flag & CFG_FLAG_FORMAT_TIMEPERIOD) != CFG_FLAG_FORMAT_NONE)
						d_integer *= (1);
				else if (UTIL_TOUPPER (*data) == 'G' && (item->flag & CFG_FLAG_FORMAT_BASE2BYTES) != CFG_FLAG_FORMAT_NONE)
						d_integer *= (1024*1024*1024);
				else if (UTIL_TOUPPER (*data) == 'M' && (item->flag & CFG_FLAG_FORMAT_BASE2BYTES) != CFG_FLAG_FORMAT_NONE)
						d_integer *= (1024*1024);
				else if (UTIL_TOUPPER (*data) == 'K' && (item->flag & CFG_FLAG_FORMAT_BASE2BYTES) != CFG_FLAG_FORMAT_NONE)
						d_integer *= (1024);
				else if (UTIL_TOUPPER (*data) == 'B' && (item->flag & CFG_FLAG_FORMAT_BASE2BYTES) != CFG_FLAG_FORMAT_NONE)
						d_integer *= (1);
				else
						return FALSE;
				data++;
		}

		if (d_negative)
				d_integer = -d_integer;

		if (item->call.c_integer != NULL && !(*item->call.c_integer) (d_integer))
				return FALSE;

		DPRINTF (("cfg_item_decode_integer: got '%ld'\n", d_integer));
		*(item->data.d_integer) = d_integer;

		return TRUE;
}

static boolean cfg_item_decode_string (/*@notnull@*/ const cfg_item_t * const item, /*@notnull@*/ const char * data)
{
		const char * d_string = data;

		assert (item != NULL);
		assert (data != NULL);

		if (item->call.c_string != NULL)
		{
				if ((item->flag & CFG_FLAG_FORMAT_CSV) != CFG_FLAG_FORMAT_NONE)
				{
						const char * d_string_ptr = d_string;
						char d_string_sub [CFG_STRING_SIZE];

						while ((d_string_ptr = util_csv_field_extract (d_string_ptr, d_string_sub, (unsigned char) sizeof (d_string_sub ))) != NULL)
						{
								if (!(*item->call.c_string) (d_string_sub))
										return FALSE;
						}
				}
				else
				{
						if (!(*item->call.c_string) (d_string))
								return FALSE;
				}
		}

		DPRINTF (("cfg_item_decode_string: got '%s'\n", d_string));
		/*@i@*/ (void) util_memcpy (*(item->data.d_string), d_string, UTIL_MIN (util_strlen (d_string) + 1, (unsigned int) sizeof (*(item->data.d_string)) - 1));
		(*item->data.d_string) [sizeof (*(item->data.d_string)) - 1] = '\0';

		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

const cfg_item_t * cfg_item_get (const cfg_config_t * const config, const char * const name)
{
		unsigned int indax;

		assert (config != NULL);
		assert (name != NULL);

		for (indax = (unsigned int) 0; indax < (unsigned int) config->item_size; indax++)
		{
				if (util_strcmpi (config->item_list [indax].name, name))
				{
						return &(config->item_list [indax]);
				}
		}

		return NULL;
}

boolean cfg_item_set (const cfg_config_t * const config, const char * const name, const char * const data)
{
		unsigned int indax;

		assert (config != NULL);
		assert (name != NULL);
		assert (data != NULL);

		for (indax = (unsigned int) 0; indax < (unsigned int) config->item_size; indax++)
		{
				const cfg_item_t * const item = &config->item_list [indax];

				assert (item != NULL);

				if (util_strcmpi (name, item->name))
				{
						if (item->type == t_boolean)
								return cfg_item_decode_boolean (item, data);
						else if (item->type == t_integer)
								return cfg_item_decode_integer (item, data);
						else if (item->type == t_string)
								return cfg_item_decode_string (item, data);
						return FALSE; /* unknown type! */
				}
		}

		return FALSE;
}

/* ----------------------------------------------------------------------------------------------------*/

static boolean cfg_item_decode (/*@notnull@*/ const cfg_config_t * const config, /*@notnull@*/ char * const string)
{
		char * name, * data;

		assert (config != NULL);
		assert (string != NULL);

		/* not quite correct: we really need tristate: TRUE, FALSE, IGNORABLE */
		if (!cfg_item_decode_detokenise (string, &name, &data))
				return TRUE;

		DPRINTF (("cfg_item_decode: parsing tokens: <%s>, <%s>\n", name, data));

		return cfg_item_set (config, name, data);
}

/* ----------------------------------------------------------------------------------------------------*/

static void cfg_item_encode (/*@notnull@*/ const cfg_item_t * const item, /*@notnull@*/ char * const buffer)
{
		assert (item != NULL);
		assert (buffer != NULL);

		(void) util_strcat (buffer, "\r\n# name: ");
		(void) util_strcat (buffer, item->name);
		if (item->description_name != NULL)
		{
				(void) util_strcat (buffer, " - ");
				(void) util_strcat (buffer, item->description_name);
		}

		(void) util_strcat (buffer, "\r\n# type: ");
		(void) util_strcat (buffer, cfg_type_string (item->type));
		if (item->description_type != NULL)
		{
				(void) util_strcat (buffer, " - ");
				(void) util_strcat (buffer, item->description_type);
		}

		(void) util_strcat (buffer, "\r\n");
		(void) util_strcat (buffer, item->name);
		(void) util_strcat (buffer, " = ");

		if (item->type == t_boolean)
				(void) util_strcat (buffer, (*(item->data.d_boolean) == TRUE) ? "true" : "false");
		else if (item->type == t_integer)
				(void) util_stratol (util_strend (buffer), *(item->data.d_integer));
		else if (item->type == t_string)
				(void) util_strcat (buffer, *(item->data.d_string));

		(void) util_strcat (buffer, "\r\n");
}

/* ----------------------------------------------------------------------------------------------------*/

static boolean cfg_decode (/*@notnull@*/ const cfg_config_t * const config, /*@notnull@*/ char * const buffer)
{
		char * ptr_begin = buffer, * ptr_end;

		assert (config != NULL);
		assert (buffer != NULL);

		while (*ptr_begin != '\0')
		{
				while (*ptr_begin != '\0' && (*ptr_begin == '\r' || *ptr_begin == '\n'))
				{
						ptr_begin++;
				}

				ptr_end = ptr_begin;

				while (*ptr_end != '\0' && !(*ptr_end == '\r' || *ptr_end == '\n'))
				{
						ptr_end++;
				}

				if (*ptr_end != '\0')
				{
						*ptr_end++ = '\0';
				}

				if (cfg_item_decode (config, ptr_begin) == FALSE)
				{
						return FALSE;
				}

				ptr_begin = ptr_end;
		}

		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

static boolean cfg_encode (/*@notnull@*/ const cfg_config_t * const config, /*@notnull@*/ char * const buffer)
{
		unsigned int indax;

		assert (config != NULL);
		assert (buffer != NULL);

		(void) util_strcpy (buffer, config->default_header);

		for (indax = (unsigned int) 0; indax < (unsigned int) config->item_size; indax++)
		{
				cfg_item_encode (&config->item_list [indax], buffer);
		}

		(void) util_strcat (buffer, config->default_footer);

		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

static boolean cfg_args (const cfg_config_t * const config, const int argc, const char ** const argv)
{
		int ac;

		assert (config != NULL);

		for (ac = 0; ac < argc && argv != NULL && argv [ac] != NULL; ac++)
		{
				if (argv [ac][0] == '-' && argv [ac][1] == '-' && argv [ac][2] == 'c' && argv [ac][3] == 'f' && argv [ac][4] == 'g')
				{
						char temp [CFG_STRING_SIZE + CFG_STRING_SIZE];

						(void) util_strcpy (temp, &argv [ac][6]);

						if (argv [ac][5] == '_')
						{
								if (cfg_decode (config, temp) == FALSE)
								{
										return FALSE;
								}
						}
						else if (argv [ac][5] == '-')
						{
								if (util_strcmpi ("nowrite", temp))
								{
										cfg_config_write = FALSE;
								}
						}
				}
		}

		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

boolean cfg_init (const cfg_config_t * const config, const int argc, const char ** const argv)
{
		fat_handle_t handle;
		char * buffer = (char *) util_buffer_get ();
		unsigned int length = util_buffer_len ();
		unsigned int actual;

		assert (config != NULL);
		assert (config->filename != NULL);
		assert (config->item_list != NULL);
		assert (config->item_size > (unsigned char) 0);

		handle = fat_open (config->filename, FAT_OPT_READ);
		if (!FAT_HANDLE_VALID (handle))
		{
				if (cfg_encode (config, buffer) == FALSE)
				{
						return FALSE;
				}

				if (cfg_args (config, argc, argv) == FALSE)
				{
						return FALSE;
				}

				if (cfg_config_write)
				{
						length = util_strlen (buffer);

						DPRINTF (("cfg_init: default config; filename = %s, length = %u\n", config->filename, length));

						handle = fat_open (config->filename, FAT_OPT_WRITE);
						if (!FAT_HANDLE_VALID (handle))
						{
								return FALSE;
						}
		
						if (fat_write (handle, (unsigned char *) buffer, length, &actual) != FAT_ERR_NONE || actual != length)
						{
								return FALSE;
						}

						if (fat_close (handle) != FAT_ERR_NONE)
						{
								return FALSE;
						}
				}
		}
		else
		{
				if (fat_read (handle, (unsigned char *) buffer, length, &actual) != FAT_ERR_NONE)
				{
						return FALSE;
				}

				if (fat_close (handle) != FAT_ERR_NONE)
				{
						return FALSE;
				}

				buffer [actual] = '\0';

				DPRINTF (("cfg_init: loaded config; filename = %s, length = %u\n", config->filename, actual));

				if (cfg_decode (config, buffer) == FALSE)
				{
						return FALSE;
				}

				if (cfg_args (config, argc, argv) == FALSE)
				{
						return FALSE;
				}
		}

		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

#ifdef TEST_ENABLED

static boolean cfg_test_call__boolean (const boolean data)
{
		return (data || !data);
}

static boolean cfg_test_call__integer (const signed long data)
{
		return (data >= (signed long) (-1024*1024) && data <= (signed long) (1024*1024));
}

static boolean cfg_test_call__string (const char * const data)
{
		return util_strcmpix (data, "dataA|dataB|otherC", '|');
}

/*@-stringliteralsmaller@*/
static cfg_data_boolean_t cfg_data_test_boolean = TRUE;
static cfg_data_integer_t cfg_data_test_integer = (signed long) 64;
static cfg_data_string_t  cfg_data_test_string = "dataA";
/*@=stringliteralsmaller@*/

static const cfg_item_t cfg_test_item_list [] =
{
		{ "test_boolean", "test option boolean", "test description boolean",
			t_boolean, CFG_FLAG_FORMAT_NONE, .call.c_boolean = cfg_test_call__boolean, .data.d_boolean = &cfg_data_test_boolean },
		{ "test_integer", "test option integer", "test description integer",
			t_integer, CFG_FLAG_FORMAT_BASE2BYTES, .call.c_integer = cfg_test_call__integer, .data.d_integer = &cfg_data_test_integer },
		{ "test_string", NULL, NULL,
			t_string, CFG_FLAG_FORMAT_NONE, .call.c_string = cfg_test_call__string, .data.d_string = &cfg_data_test_string }
};

static const cfg_config_t cfg_test_config =
{
		"TESTCONF.TMP",
		"# header\n",
		"# footer\n",
		cfg_test_item_list,
		(unsigned char) (sizeof (cfg_test_item_list) / sizeof (cfg_test_item_list [0]))
};

#define cfg_data_get(c,n) (&(cfg_item_get(c,n)->data))

test_result_t cfg_test (void)
{
		char * buffer = (char *) util_buffer_alloc ((unsigned int) 8192);
		char * name, * data;

		test_assert (buffer != NULL);

		/* config encode/decode */
		test_assert (cfg_encode (&cfg_test_config, buffer) == TRUE);
		test_assert (cfg_decode (&cfg_test_config, buffer) == TRUE);

		/* config item decode detokenise */
		(void) util_strcpy (buffer, "");
		test_assert (cfg_item_decode_detokenise (buffer, &name, &data) == FALSE);
		(void) util_strcpy (buffer, "	 ");
		test_assert (cfg_item_decode_detokenise (buffer, &name, &data) == FALSE);
		(void) util_strcpy (buffer, "	 # comment");
		test_assert (cfg_item_decode_detokenise (buffer, &name, &data) == FALSE);
		(void) util_strcpy (buffer, "name");
		test_assert (cfg_item_decode_detokenise (buffer, &name, &data) == FALSE);
		(void) util_strcpy (buffer, "name=");
		test_assert (cfg_item_decode_detokenise (buffer, &name, &data) == FALSE);
		(void) util_strcpy (buffer, "name=#");
		test_assert (cfg_item_decode_detokenise (buffer, &name, &data) == FALSE);
		(void) util_strcpy (buffer, "=data");
		test_assert (cfg_item_decode_detokenise (buffer, &name, &data) == FALSE);
		(void) util_strcpy (buffer, "name=data");
		test_assert (cfg_item_decode_detokenise (buffer, &name, &data) == TRUE);
		test_assert (util_strcmpi (name, "name") && util_strcmpi (data, "data"));
		(void) util_strcpy (buffer, "name=data # comment");
		test_assert (cfg_item_decode_detokenise (buffer, &name, &data) == TRUE);
		test_assert (util_strcmpi (name, "name") && util_strcmpi (data, "data"));
		(void) util_strcpy (buffer, "name=data# comment");
		test_assert (cfg_item_decode_detokenise (buffer, &name, &data) == TRUE);
		test_assert (util_strcmpi (name, "name") && util_strcmpi (data, "data"));
		(void) util_strcpy (buffer, "	 name=data");
		test_assert (cfg_item_decode_detokenise (buffer, &name, &data) == TRUE);
		test_assert (util_strcmpi (name, "name") && util_strcmpi (data, "data"));
		(void) util_strcpy (buffer, "	 name  =  data");
		test_assert (cfg_item_decode_detokenise (buffer, &name, &data) == TRUE);
		test_assert (util_strcmpi (name, "name") && util_strcmpi (data, "data"));

		/* config item decode boolean */
		test_assert (cfg_item_decode_boolean (&cfg_test_item_list [0], "a") == FALSE);
		test_assert (cfg_item_decode_boolean (&cfg_test_item_list [0], "t") == TRUE);
		test_assert (cfg_item_decode_boolean (&cfg_test_item_list [0], "f") == TRUE);
		test_assert (cfg_item_decode_boolean (&cfg_test_item_list [0], "true") == TRUE);
		test_assert (*cfg_data_get (&cfg_test_config, "test_boolean")->d_boolean == TRUE);
		test_assert (cfg_item_decode_boolean (&cfg_test_item_list [0], "false") == TRUE);
		test_assert (*cfg_data_get (&cfg_test_config, "test_boolean")->d_boolean == FALSE);

		/* config item decode integer */
		test_assert (cfg_item_decode_integer (&cfg_test_item_list [1], "-1") == TRUE);
		test_assert (cfg_item_decode_integer (&cfg_test_item_list [1], "1048577") == FALSE);
		test_assert (cfg_item_decode_integer (&cfg_test_item_list [1], "a") == FALSE);
		test_assert (cfg_item_decode_integer (&cfg_test_item_list [1], "0") == TRUE);
		test_assert (*cfg_data_get (&cfg_test_config, "test_integer")->d_integer == (signed long) (0));
		test_assert (cfg_item_decode_integer (&cfg_test_item_list [1], "1048576") == TRUE);
		test_assert (*cfg_data_get (&cfg_test_config, "test_integer")->d_integer == (signed long) (1048576));
		test_assert (cfg_item_decode_integer (&cfg_test_item_list [1], "1024b") == TRUE);
		test_assert (*cfg_data_get (&cfg_test_config, "test_integer")->d_integer == (signed long) (1024));
		test_assert (cfg_item_decode_integer (&cfg_test_item_list [1], "1024k") == TRUE);
		test_assert (*cfg_data_get (&cfg_test_config, "test_integer")->d_integer == (signed long) (1024*1024));
		test_assert (cfg_item_decode_integer (&cfg_test_item_list [1], "1m") == TRUE);
		test_assert (*cfg_data_get (&cfg_test_config, "test_integer")->d_integer == (signed long) (1024*1024));
		test_assert (cfg_item_decode_integer (&cfg_test_item_list [1], "-1m") == TRUE);
		test_assert (*cfg_data_get (&cfg_test_config, "test_integer")->d_integer == (signed long) (-1024*1024));

		/* config item decode string */
		test_assert (cfg_item_decode_string (&cfg_test_item_list [2], "foobar") == FALSE);
		test_assert (cfg_item_decode_string (&cfg_test_item_list [2], "dataA") == TRUE);
		test_assert (util_strcmpi (*cfg_data_get (&cfg_test_config, "test_string")->d_string, "dataA"));
		test_assert (cfg_item_decode_string (&cfg_test_item_list [2], "dataB") == TRUE);
		test_assert (util_strcmpi (*cfg_data_get (&cfg_test_config, "test_string")->d_string, "dataB"));
		test_assert (cfg_item_decode_string (&cfg_test_item_list [2], "dataC") == FALSE);
		test_assert (cfg_item_decode_string (&cfg_test_item_list [2], "otherC") == TRUE);
		test_assert (util_strcmpi (*cfg_data_get (&cfg_test_config, "test_string")->d_string, "otherC"));
		test_assert (cfg_item_decode_string (&cfg_test_item_list [2], "data") == FALSE);

		util_buffer_reset ();

		return TEST_RESULT_OKAY;
}

#endif

/* ----------------------------------------------------------------------------------------------------*/
