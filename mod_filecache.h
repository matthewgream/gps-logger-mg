
/* ----------------------------------------------------------------------------------------------------*/
/*
	mod_filecache.h: gps logger module for file cache processing.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#ifndef MOD_FILECACHE_H_
#define MOD_FILECACHE_H_

/* ----------------------------------------------------------------------------------------------------*/

typedef struct
{
		unsigned int buffer_size;
		unsigned long timeout_normal;
		unsigned long timeout_quiet;
		/*@null@*/ void (*notify_write) (void);
}
cachefile_config_t;

/* ----------------------------------------------------------------------------------------------------*/

boolean cachefile_init (/*@notnull@*/ const cachefile_config_t * const config);
boolean cachefile_term (void);

boolean cachefile_attach (fat_handle_t handle);
boolean cachefile_detach (void);

boolean cachefile_write (/*@notnull@*/ const unsigned char * const buffer, const unsigned int length);

#ifdef TEST_ENABLED
test_result_t cachefile_test (void);
#endif

/* ----------------------------------------------------------------------------------------------------*/

#endif /*MOD_FILECACHE_H_*/

