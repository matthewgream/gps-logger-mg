
/* ----------------------------------------------------------------------------------------------------*/
/*
	mod_filefatfs.h: gps logger module for FAT16 file system.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#ifndef MOD_FILEFATFS_H_
#define MOD_FILEFATFS_H_

/* ----------------------------------------------------------------------------------------------------*/

#define FAT_OPT_NONE					((fat_options_t) 0x00)
#define FAT_OPT_READ					((fat_options_t) 0x01)
#define FAT_OPT_WRITE					((fat_options_t) 0x02)
#define FAT_OPT_SYNC					((fat_options_t) 0x04)
#define FAT_OPT_APPEND					((fat_options_t) 0x08)

#define FAT_ERR_NONE					((fat_error_t) 0)
#define FAT_ERR_CARDINITFAILED			((fat_error_t) -1)
#define FAT_ERR_CARDENABLEFAILED		((fat_error_t) -2)
#define FAT_ERR_CARDCONFIGFAILED		((fat_error_t) -3)
#define FAT_ERR_CARDNOTPRESENT			((fat_error_t) -4)
#define FAT_ERR_CARDWRITEFAILED			((fat_error_t) -5)
#define FAT_ERR_CARDREADFAILED			((fat_error_t) -6)
#define FAT_ERR_CARDFULL				((fat_error_t) -7)
#define FAT_ERR_FILESYSUNKNOWN			((fat_error_t) -8)
#define FAT_ERR_FILESYSDIRECTORYFULL	((fat_error_t) -9)
#define FAT_ERR_FILESYSCLUSTERSFULL		((fat_error_t) -10)
#define FAT_ERR_FILEHANDLEINVALID		((fat_error_t) -11)
#define FAT_ERR_ACCESSMODEINVALID		((fat_error_t) -12)
#define FAT_ERR_FILENAMEINVALID			((fat_error_t) -13)
#define FAT_ERR_FILENOTFOUND			((fat_error_t) -14)
#define FAT_ERR_FILEALREADYEXISTS		((fat_error_t) -15)
#define FAT_ERR_FILEOPENFORREADALREADY	((fat_error_t) -16)
#define FAT_ERR_FILEOPENFORWRITEALREADY ((fat_error_t) -17)
#define FAT_ERR_FILENAMENOTSUPPORTED	((fat_error_t) -18)
#define FAT_ERR_FILENAMETOOLONG			((fat_error_t) -19)
#define FAT_ERR_TRANSFERSIZEINVALID		((fat_error_t) -20)
#define FAT_ERR_ENDOFFILE				((fat_error_t) -21)
#define FAT_ERR_CONFIGINVALID			((fat_error_t) -22)
#define FAT_ERR_OPTIONSINVALID			((fat_error_t) -23)
#define FAT_ERR_TRANSFERDATAINVALID		((fat_error_t) -24)
#define FAT_ERR_CARDRESUMEFAILED		((fat_error_t) -25)
#define FAT_ERR_CARDSUSPENDFAILED		((fat_error_t) -26)
#define FAT_ERR_FILESREMAINOPEN			((fat_error_t) -27)
#define FAT_ERR_SEEKINVALID				((fat_error_t) -28)
#define FAT_ERR_FILEINCONSISTENCY		((fat_error_t) -29)
#define FAT_ERR_NOTSUPPORTED			((fat_error_t) -30)
#define FAT_ERR_ENDOFDIRECTORY			((fat_error_t) -31)

/* ----------------------------------------------------------------------------------------------------*/

typedef signed char fat_handle_t;
typedef signed char fat_error_t;
typedef unsigned char fat_options_t;

#define FAT_HANDLE_INVALID				((fat_handle_t) -1)
#define FAT_HANDLE_VALID(h)				((h) >= (fat_handle_t) 0)

typedef struct
{
		unsigned long state; /* don't touch */
		char name [13];
		unsigned long size;
}
fat_dirent_t;

typedef struct
{
}
fat_config_t;

/* ----------------------------------------------------------------------------------------------------*/

fat_error_t fat_init (/*@notnull@*/ const fat_config_t * const config);
fat_error_t fat_term (void);

fat_error_t fat_suspend (void);
fat_error_t fat_resume (void);

fat_error_t fat_dirent_init (/*@notnull@*/ /*@out@*/ fat_dirent_t* const dirent);
fat_error_t fat_dirent_next (/*@notnull@*/ fat_dirent_t* const dirent);

fat_handle_t fat_open (/*@notnull@*/ const char * const filename, const fat_options_t fileopts);
fat_error_t fat_close (fat_handle_t handle);

fat_error_t fat_read (fat_handle_t handle, /*@notnull@*/ /*@out@*/ unsigned char * const buffer, const unsigned int length, /*@null@*/ /*@out@*/ unsigned int * const actual);
fat_error_t fat_write (fat_handle_t handle, /*@notnull@*/ const unsigned char * const buffer, const unsigned int length, /*@null@*/ /*@out@*/ unsigned int * const actual);

fat_error_t fat_tell (fat_handle_t handle, /*@notnull@*/ /*@out@*/ unsigned long * const filesize);

fat_error_t fat_sync (fat_handle_t handle);

fat_error_t fat_flush (void);

fat_error_t fat_unlink (/*@notnull@*/ const char * const filename);

#ifdef TEST_ENABLED
test_result_t fat_test (void);
#endif

/* ----------------------------------------------------------------------------------------------------*/

#endif /*MOD_FILEFATFS_H_*/

