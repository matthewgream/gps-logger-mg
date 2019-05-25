
/* ----------------------------------------------------------------------------------------------------*/
/*
	mod_packlzari.h: gps logger module for LZARI based packer.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#ifndef MOD_PACKLZARI_H_
#define MOD_PACKLZARI_H_

/* ----------------------------------------------------------------------------------------------------*/

typedef boolean (*packlzari_output_handler_t) (const unsigned char* const, const unsigned int);

#ifdef PACKLZARI_ENCODE

boolean packlzari_encode_init (/*@notnull@*/ packlzari_output_handler_t handler);
boolean packlzari_encode_term (void);
boolean packlzari_encode_write (/*@notnull@*/ const unsigned char * const buffer, const unsigned int length);

#endif

#ifdef PACKLZARI_DECODE

#if 0
boolean packlzari_decode_init (/*@notnull@*/ packlzari_output_handler_t handler);
boolean packlzari_decode_term (void);
boolean packlzari_decode_write (/*@notnull@*/ const unsigned char * const buffer, const unsigned int length);
#endif

#endif

#ifdef TEST_ENABLED
test_result_t packlzari_test (void);
#endif

/* ----------------------------------------------------------------------------------------------------*/

#endif /*MOD_PACKLZARI_H_*/

