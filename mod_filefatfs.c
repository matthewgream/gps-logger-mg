
/* ----------------------------------------------------------------------------------------------------*/
/*
	mod_filefatfs.c: gps logger module for FAT16 file system.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#include "common.h"
#include "mod_util.h"
#include "mod_filespisd.h"
#include "mod_filefatfs.h"

/* ----------------------------------------------------------------------------------------------------*/
/* supports FAT16 and standard 8.3 filenames only */
/* FAT32 support would be great, no need for long filenames */
/* ----------------------------------------------------------------------------------------------------*/

#define FAT_FILESYS_UNKNOWN	  ((unsigned char) 0)
#define FAT_FILESYS_FAT16	  ((unsigned char) 1)
#define FAT_FILESYS_FAT32	  ((unsigned char) 2)

#define FAT_OPTION_NONE		  ((unsigned char) 0x00)
#define FAT_OPTION_READ		  ((unsigned char) 0x01)
#define FAT_OPTION_WRITE	  ((unsigned char) 0x02)
#define FAT_OPTION_SYNC		  ((unsigned char) 0x04)
#define FAT_OPTION_ACTIVE	  ((unsigned char) 0x80)

#define FAT_CONFIG_NONE		  ((unsigned char) 0x00)

#define FAT_STATUS_NONE		  ((unsigned char) 0x00)
#define FAT_STATUS_UNSYNC	  ((unsigned char) 0x01)

#define FAT_FILE_SIZE		  ((fat_handle_t) 4)

#define FAT_SEEK_END		  ((fat_seek_t) 0)

#define FAT_PARTINFO_SECTOR	  0
#define FAT_PARTINFO_OFFSET	  0
#define FAT_PARTINFO_LENGTH	  64

#define FAT_PARTOFFS_SECTOR	  0
#define FAT_PARTOFFS_OFFSET	  454
#define FAT_PARTOFFS_LENGTH	  4

#define FAT_CLUSTER_SECTOR_TIMER_DELAY	5

/* ----------------------------------------------------------------------------------------------------*/

/* XXX: prefer if another approach was used- some sort of configurable layers */
#ifdef EMULATE
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#ifdef linux
#define FAT_EMULATE_OPEN_OFFS 5
#else
#define FAT_EMULATE_OPEN_OFFS 3
#endif

/*@i@*/ /*@null@*/ extern char * emu_file_dir;
static boolean emu_file_map [128];
#define FAT_EMULATE_HOOK_INIT() \
		{ if (emu_file_dir != NULL) { return FAT_ERR_NONE; } }
#define FAT_EMULATE_HOOK_TERM() \
		{ if (emu_file_dir != NULL) { return FAT_ERR_NONE; } }
#define FAT_EMULATE_HOOK_FILE_SEEK(handle,seek) \
		{ if (emu_file_dir != NULL) { (void) lseek ((int) handle, (off_t) 0, (seek == FAT_SEEK_END) ? SEEK_END : SEEK_SET); return FAT_ERR_NONE; } }
#define FAT_EMULATE_HOOK_SUSPEND() \
		{ if (emu_file_dir != NULL) { return FAT_ERR_NONE; } }
#define FAT_EMULATE_HOOK_RESUME() \
		{ if (emu_file_dir != NULL) { return FAT_ERR_NONE; } }
#define FAT_EMULATE_HOOK_DIRENT_INIT(direntx) \
		{ if (emu_file_dir != NULL) { \
				DIR * dirp = opendir (emu_file_dir); \
				if (dirp == NULL) return FAT_ERR_ENDOFDIRECTORY; \
				/*@-abstract@*/ direntx->state = (unsigned long) dirp; /*@=abstract@*/ \
				return FAT_ERR_NONE; \
		} }
#define FAT_EMULATE_HOOK_DIRENT_NEXT(direntx) \
		{ if (emu_file_dir != NULL) { \
				struct stat sb; char n [128]; \
				/*@-abstract@*/ struct dirent * dp = readdir ((DIR *) direntx->state ); /*@=abstract@*/ \
				/*@-abstract@*/ if (dp == NULL) { (void) closedir ((DIR *) direntx->state); return FAT_ERR_ENDOFDIRECTORY; }; /*@=abstract@*/ \
				(void) util_memset (direntx->name, (unsigned char) 0, (unsigned int) sizeof (direntx->name)); \
				(void) util_memcpy (direntx->name, dp->d_name, UTIL_MIN ((unsigned int)(sizeof (direntx->name) - 1), (unsigned int) (dp->d_namlen))); \
				(void) util_strcpy (n, emu_file_dir); (void) util_strcat (n, "/"); (void) util_strcat (n, dp->d_name); \
				direntx->size = (unsigned long) ((stat (n, &sb) == 0) ? sb.st_size : 0); \
				return FAT_ERR_NONE; \
		} }
#define FAT_EMULATE_HOOK_OPEN(filename,fileopts) \
		{ if (emu_file_dir != NULL) { \
				int f; char n [128]; (void) util_strcpy (n, emu_file_dir); (void) util_strcat (n, "/"); (void) util_strcat (n, filename); \
				f = ((fileopts & FAT_OPT_READ) != FAT_OPT_NONE) ? O_RDONLY|O_NONBLOCK : O_WRONLY|O_CREAT|O_EXCL|O_NONBLOCK; \
				if ((fileopts & FAT_OPT_APPEND) != FAT_OPT_NONE) { f &= ~O_EXCL; f |= O_APPEND; } \
				/*@-unrecog@*/ f = open (n, f, 0655); \
				if (f < 0) return FAT_ERR_FILENOTFOUND; \
				if (f > ((int) FAT_FILE_SIZE + FAT_EMULATE_OPEN_OFFS)) { (void) close (f); if ((fileopts & FAT_OPT_WRITE) == FAT_OPT_WRITE) (void) unlink (n); return FAT_ERR_FILEHANDLEINVALID; } \
				if (flock (f, LOCK_EX|LOCK_NB) != 0) { (void) close (f); if ((fileopts & FAT_OPT_WRITE) == FAT_OPT_WRITE) \
						{ (void) unlink (n); return FAT_ERR_FILEOPENFORWRITEALREADY; } return FAT_ERR_FILEOPENFORREADALREADY; } /*@=unrecog@*/ \
				emu_file_map [f] = TRUE; \
				return (fat_handle_t) f; \
		} }
#define FAT_EMULATE_HOOK_READ(handle,buffer,length,actual) \
		{ if (emu_file_dir != NULL) { \
				unsigned int a; \
				if (emu_file_map [handle] != TRUE) return FAT_ERR_FILEHANDLEINVALID; \
				if (buffer == NULL) return FAT_ERR_TRANSFERDATAINVALID; \
				if (length == 0) return FAT_ERR_TRANSFERSIZEINVALID; \
				a = (unsigned int) read ((int) handle, (void *) buffer, (size_t) length); \
				if (actual != NULL) *actual = a; return FAT_ERR_NONE; } }
#define FAT_EMULATE_HOOK_WRITE(handle,buffer,length,actual) \
		{ if (emu_file_dir != NULL) { \
				unsigned int a; \
				if (emu_file_map [handle] != TRUE) return FAT_ERR_FILEHANDLEINVALID; \
				if (buffer == NULL) return FAT_ERR_TRANSFERDATAINVALID; \
				if (length == 0) return FAT_ERR_TRANSFERSIZEINVALID; \
				a = (unsigned int) write ((int) handle, (const void *) buffer, (size_t) length); \
				if (actual != NULL) *actual = a; return FAT_ERR_NONE; } }
#define FAT_EMULATE_HOOK_CLOSE(handle) \
		{ if (emu_file_dir != NULL) { \
				emu_file_map [handle] = FALSE; \
				(void) close ((int) handle); return FAT_ERR_NONE; } }
#define FAT_EMULATE_HOOK_TELL(handle,filesize) \
		{ if (emu_file_dir != NULL) { *filesize = (unsigned long) lseek ((int) handle, (off_t) 0, SEEK_CUR); return FAT_ERR_NONE; } }
#define FAT_EMULATE_HOOK_SYNC(handle) \
		{ if (emu_file_dir != NULL) { return FAT_ERR_NONE; } }
#define FAT_EMULATE_HOOK_UNLINK(filename) \
		{ if (emu_file_dir != NULL) { \
				char n [128]; (void) util_strcpy (n, emu_file_dir); (void) util_strcat (n, "/"); (void) util_strcat (n, filename); \
				(void) unlink (n); return FAT_ERR_NONE; } }
#define FAT_EMULATE_HOOK_FLUSH() \
		{ if (emu_file_dir != NULL) { return FAT_ERR_NONE; } }
#else
#define FAT_EMULATE_HOOK_INIT()
#define FAT_EMULATE_HOOK_TERM()
#define FAT_EMULATE_HOOK_FILE_SEEK(handle,seek)
#define FAT_EMULATE_HOOK_SUSPEND()
#define FAT_EMULATE_HOOK_RESUME()
#define FAT_EMULATE_HOOK_DIRENT_INIT(dirent)
#define FAT_EMULATE_HOOK_DIRENT_NEXT(dirent)
#define FAT_EMULATE_HOOK_OPEN(filename,fileopts)
#define FAT_EMULATE_HOOK_READ(handle,buffer,length,actual)
#define FAT_EMULATE_HOOK_WRITE(handle,buffer,length,actual)
#define FAT_EMULATE_HOOK_CLOSE(handle)
#define FAT_EMULATE_HOOK_TELL(handle,filesize)
#define FAT_EMULATE_HOOK_SYNC(handle)
#define FAT_EMULATE_HOOK_UNLINK(filename)
#define FAT_EMULATE_HOOK_FLUSH()
#endif

/* ----------------------------------------------------------------------------------------------------*/

typedef unsigned short fat_cluster_t;
typedef unsigned long fat_sector_t;
typedef unsigned long fat_filesize_t;
typedef unsigned int fat_offset_t;
typedef unsigned char fat_status_t;
typedef unsigned char fat_seek_t;

/* ----------------------------------------------------------------------------------------------------*/

static unsigned char fat_conf;

/* ----------------------------------------------------------------------------------------------------*/

typedef struct
{
		fat_handle_t		handle;
		unsigned char		options;
		fat_sector_t		dirSector;
		fat_offset_t		dirOffset;
		fat_cluster_t		fileCluster;
		fat_filesize_t		fileSize;
		fat_cluster_t		currentCluster;
		fat_sector_t		currentSector;
		fat_offset_t		currentOffset;
		fat_status_t		status;
		fat_sector_t		cachedSector;
		unsigned char		cachedBuffer [CARD_SECTOR_SIZE];
}
fat_file_t;

static fat_file_t fat_file [FAT_FILE_SIZE];

/* ----------------------------------------------------------------------------------------------------*/

typedef struct
{
		unsigned char		fileSys;
		fat_sector_t		sectorsOffset;
		unsigned int		sectorsPerCluster;
		unsigned int		sectorsReserved;
		unsigned int		sectorsPerFAT;
		fat_sector_t		sectorsTotal;
		unsigned char		numberFAT;
		fat_offset_t		cachedFATsector_allocateLastOffset;
		fat_sector_t		cachedFATsector;
		unsigned char		cachedFATbuffer [CARD_SECTOR_SIZE];
		boolean				cachedFATbuffer_dirty;
		volatile boolean	cachedFATbuffer_expired;
		util_timer_handle_t cachedFATbuffer_handleTimer;
		util_scheduler_handle_t cachedFATbuffer_handlePoller;
		fat_sector_t		rootDirectory;
		unsigned int		rootSectors;
		fat_sector_t		cachedDIRsector;
		unsigned char		cachedDIRbuffer [CARD_SECTOR_SIZE];
		fat_sector_t		dataStarts;
		fat_cluster_t		clustersMax;
}
fat_disk_t;

static fat_disk_t fat_disk;

/* ----------------------------------------------------------------------------------------------------*/

#define FAT_CHECK_CARD					if (card_detect () == FALSE) return FAT_ERR_CARDNOTPRESENT;
#define FAT_CHECK_FILESYS				if (fat_disk.fileSys == FAT_FILESYS_UNKNOWN) return FAT_ERR_FILESYSUNKNOWN;
#define FAT_CHECK_HANDLE(handle)		if (handle == NULL) return FAT_ERR_FILEHANDLEINVALID;
#define FAT_CHECK_MODE(options,mode)	if ((options & (mode)) == FAT_OPTION_NONE) return FAT_ERR_ACCESSMODEINVALID;
#define FAT_CHECK_TRANSFER_DATA(buff)	if (buff == NULL) return FAT_ERR_TRANSFERDATAINVALID;
#define FAT_CHECK_TRANSFER_SIZE(size)	if (size == 0) return FAT_ERR_TRANSFERSIZEINVALID;
#define FAT_CHECK_FILENAME(name)		if (name == NULL || *name == '\0') return FAT_ERR_FILENAMEINVALID;
#define FAT_CHECK_FATCONFIG(config)		if (config == NULL) return FAT_ERR_CONFIGINVALID;
#define FAT_CHECK_FATOPTIONS(options)	if ((options & ~(FAT_OPT_READ|FAT_OPT_WRITE|FAT_OPT_SYNC|FAT_OPT_APPEND)) != FAT_OPT_NONE) return FAT_ERR_OPTIONSINVALID;

/* ----------------------------------------------------------------------------------------------------*/

#define FAT_IS_DSK_SECTOR_VALID(s) \
		((s) >= (fat_sector_t) fat_disk.sectorsReserved && (s) < (fat_sector_t) (fat_disk.sectorsReserved + fat_disk.sectorsPerFAT))
#define FAT_IS_DIR_SECTOR_VALID(s) \
		((s) >= fat_disk.rootDirectory && (s) < (fat_disk.rootDirectory + fat_disk.rootSectors))
#define FAT_IS_DIR_OFFSET_VALID(o) \
		((o) < (fat_offset_t) CARD_SECTOR_SIZE)
#define FAT_IS_FIL_SECTOR_VALID(s) \
		((s) >= fat_disk.dataStarts && (s) < (fat_sector_t) (fat_disk.sectorsPerCluster * (fat_disk.sectorsPerFAT << 8)))
#define FAT_IS_CLUSTER_VALID(c) \
		((c) >= (fat_cluster_t) 0x0002 && (c) <= (fat_cluster_t) 0xFFF6 && (c) < (fat_cluster_t) (fat_disk.sectorsPerFAT << 8))

/* ----------------------------------------------------------------------------------------------------*/

static fat_error_t fat_filesys_getpartoffset (/*@unique@*/ /*@notnull@*/ /*@out@*/ unsigned char * buffer, /*@notnull@*/ fat_sector_t * const sector)
{
		assert (sector != NULL);
		assert (FAT_PARTOFFS_LENGTH <= CARD_SECTOR_SIZE);

		if (card_sector_read (FAT_PARTOFFS_SECTOR, buffer) != (unsigned int) CARD_SECTOR_SIZE)
		{
				return FAT_ERR_CARDREADFAILED;
		}

		*sector = ((fat_sector_t) buffer [FAT_PARTOFFS_OFFSET + 0]) | ((fat_sector_t) buffer [FAT_PARTOFFS_OFFSET + 1] << 8) |
				  ((fat_sector_t) buffer [FAT_PARTOFFS_OFFSET + 2] << 16) | ((fat_sector_t) buffer [FAT_PARTOFFS_OFFSET + 3] << 24);

		return FAT_ERR_NONE;
}

/* ----------------------------------------------------------------------------------------------------*/

static fat_error_t fat_filesys_load (/*@unique@*/ /*@out@*/ /*@notnull@*/ unsigned char * const buffer, /*@out@*/ /*@notnull@*/ unsigned char * const partinfo)
{
		assert (buffer != NULL);
		assert (partinfo != NULL);
		assert (FAT_PARTINFO_LENGTH <= CARD_SECTOR_SIZE);

		if (card_sector_read (fat_disk.sectorsOffset + FAT_PARTINFO_SECTOR, buffer) != (unsigned int) CARD_SECTOR_SIZE)
		{
				return FAT_ERR_CARDREADFAILED;
		}
		(void) util_memcpy (partinfo, &buffer [FAT_PARTINFO_OFFSET], (unsigned int) FAT_PARTINFO_LENGTH);

		return FAT_ERR_NONE;
}

/* ----------------------------------------------------------------------------------------------------*/

static fat_error_t fat_filesys_identify (/*@notnull@*/ const unsigned char * const partinfo)
{
		assert (partinfo != NULL);

		fat_disk.fileSys = FAT_FILESYS_UNKNOWN;

		if ((partinfo [54] == (unsigned char) 'F') && (partinfo [55] == (unsigned char) 'A') && (partinfo [56] == (unsigned char) 'T') &&
						(partinfo [57] == (unsigned char) '1') && (partinfo [58] == (unsigned char) '6'))
		{
				fat_disk.fileSys = FAT_FILESYS_FAT16;

				return FAT_ERR_NONE;
		}

		return FAT_ERR_FILESYSUNKNOWN;
}

/* ----------------------------------------------------------------------------------------------------*/

static fat_error_t fat_filesys_getinfo (/*@notnull@*/ const unsigned char * const partinfo)
{
		assert (partinfo != NULL);

		if (fat_disk.fileSys == FAT_FILESYS_FAT16)
		{
				fat_disk.sectorsPerCluster = (unsigned int) partinfo [13];
				fat_disk.sectorsReserved = (unsigned int) partinfo [14] | ((unsigned int) partinfo [15] << 8);
				fat_disk.numberFAT = (unsigned char) partinfo [16];
				fat_disk.rootSectors = ( ((unsigned int) partinfo [17] | ((unsigned int) partinfo [18] << 8)) / CARD_SECTOR_SIZE ) * 32;
				fat_disk.sectorsTotal = (fat_sector_t) partinfo [19] | ((fat_sector_t) partinfo [20] << 8);
				fat_disk.sectorsPerFAT = (unsigned int) partinfo [22] | ((unsigned int) partinfo [23] << 8);
				fat_disk.rootDirectory = (fat_sector_t) fat_disk.sectorsReserved + (fat_sector_t) (fat_disk.sectorsPerFAT * (unsigned int) fat_disk.numberFAT);
				fat_disk.dataStarts = (fat_sector_t) fat_disk.rootDirectory + (fat_sector_t) fat_disk.rootSectors;
				if (fat_disk.sectorsTotal == 0)
				{
						fat_disk.sectorsTotal = (unsigned long) partinfo [32] | ((unsigned long) partinfo [33] << 8) |
												((unsigned long) partinfo [34] << 16) | ((unsigned long) partinfo [35] << 24);
				}
				fat_disk.clustersMax = (fat_cluster_t) ((((fat_disk.sectorsTotal - fat_disk.dataStarts) / fat_disk.sectorsPerCluster) + 0x0002) - 2);
		}

		return FAT_ERR_NONE;
}

/* ----------------------------------------------------------------------------------------------------*/

static fat_error_t fat_cluster_sector_sync (void)
{
		if (fat_disk.cachedFATbuffer_dirty == TRUE)
		{
				unsigned int fatIndex;

				for (fatIndex = (unsigned int) 0; fatIndex < (unsigned int) fat_disk.numberFAT; fatIndex++)
				{
						if (card_sector_write (fat_disk.sectorsOffset + fat_disk.cachedFATsector + (fatIndex * fat_disk.sectorsPerFAT), fat_disk.cachedFATbuffer)
										!= (unsigned int) CARD_SECTOR_SIZE)
						{
								return FAT_ERR_CARDWRITEFAILED;
						}
				}

				fat_disk.cachedFATbuffer_dirty = FALSE;
		}

		return FAT_ERR_NONE;
}

static fat_error_t fat_cluster_sector_load (const fat_sector_t sector)
{
		assert (FAT_IS_DSK_SECTOR_VALID (sector));

		if (fat_disk.cachedFATsector != sector)
		{
				fat_error_t result;

				if ((result = fat_cluster_sector_sync ()) != FAT_ERR_NONE)
				{
						return result;
				}

				fat_disk.cachedFATsector = sector;
				if (card_sector_read (fat_disk.sectorsOffset + fat_disk.cachedFATsector, fat_disk.cachedFATbuffer) != (unsigned int) CARD_SECTOR_SIZE)
				{
						return FAT_ERR_CARDREADFAILED;
				}
		}

		return FAT_ERR_NONE;
}

static fat_error_t fat_cluster_sector_save (void)
{
		fat_disk.cachedFATbuffer_dirty = TRUE;

		if (UTIL_TIMER_HANDLE_VALID (fat_disk.cachedFATbuffer_handleTimer))
		{
				util_timer_restart (fat_disk.cachedFATbuffer_handleTimer);
		}

		return FAT_ERR_NONE;
}

static boolean fat_cluster_sector_handler_scheduler (void * const token)
{
		UNUSED (token);

		(void) card_state_resume ();

		/* ignore errors, FAT will remain temporarly not-updated */
		(void) fat_cluster_sector_sync ();

		(void) card_state_suspend ();

		return TRUE;
}

static void fat_cluster_sector_handler_timer (void * const token)
{
		UNUSED (token);

		fat_disk.cachedFATbuffer_expired = TRUE;
}

static void fat_cluster_sector_init (void)
{
/*@-evalorderuncon@*/
		fat_disk.cachedFATbuffer_handlePoller = util_scheduler_create (fat_cluster_sector_handler_scheduler, NULL, &fat_disk.cachedFATbuffer_expired);
		assert (UTIL_SCHEDULER_HANDLE_VALID (fat_disk.cachedFATbuffer_handlePoller));
		fat_disk.cachedFATbuffer_handleTimer = util_timer_create (fat_cluster_sector_handler_timer, NULL, (unsigned long) FAT_CLUSTER_SECTOR_TIMER_DELAY);
		assert (UTIL_TIMER_HANDLE_VALID (fat_disk.cachedFATbuffer_handleTimer));
/*@=evalorderuncon@*/
}

static void fat_cluster_sector_term (void)
{
		util_timer_destroy (fat_disk.cachedFATbuffer_handleTimer);
		util_scheduler_destroy (fat_disk.cachedFATbuffer_handlePoller);
}

/* ----------------------------------------------------------------------------------------------------*/

static fat_sector_t fat_cluster_to_sector (const fat_cluster_t cluster)
{
		assert (FAT_IS_CLUSTER_VALID (cluster));

		return ((cluster - 0x0002) * fat_disk.sectorsPerCluster) + fat_disk.dataStarts;
}

/* ----------------------------------------------------------------------------------------------------*/

static boolean fat_cluster_valid (const fat_cluster_t cluster)
{
		return (cluster >= (fat_cluster_t) 0x0002 && cluster <= (fat_cluster_t) 0xFFF6) ? TRUE : FALSE;
}

/* ----------------------------------------------------------------------------------------------------*/

static fat_error_t fat_cluster_allocate (/*@notnull@*/ fat_cluster_t * const cluster)
{
		fat_offset_t clusterOffset;
		fat_offset_t sectorOffset;
		fat_error_t result;

		assert (cluster != NULL);

		for (sectorOffset = 0; sectorOffset < fat_disk.sectorsPerFAT; sectorOffset++)
		{
				boolean clustersValid = TRUE;

				if ((result = fat_cluster_sector_load ((fat_sector_t) (fat_disk.sectorsReserved + fat_disk.cachedFATsector_allocateLastOffset))) != FAT_ERR_NONE)
				{
						return result;
				}

				for (clusterOffset = 0; clusterOffset < (fat_offset_t) CARD_SECTOR_SIZE && clustersValid; clusterOffset += (fat_offset_t) 2)
				{
						fat_cluster_t clusterTemp = (fat_cluster_t) (fat_disk.cachedFATsector_allocateLastOffset << 8) | (fat_cluster_t) (clusterOffset >> 1);

						if (clusterTemp > fat_disk.clustersMax)
						{
								clustersValid = FALSE;
						}
						else if (fat_disk.cachedFATbuffer [clusterOffset + 0] == (unsigned char) 0x00 && fat_disk.cachedFATbuffer [clusterOffset + 1] == (unsigned char) 0x00)
						{
								fat_disk.cachedFATbuffer [clusterOffset + 0] = (unsigned char) 0xFF;
								fat_disk.cachedFATbuffer [clusterOffset + 1] = (unsigned char) 0xFF;

								*cluster = clusterTemp;

								return fat_cluster_sector_save ();
						}
				}

				if (++fat_disk.cachedFATsector_allocateLastOffset == fat_disk.sectorsPerFAT)
				{
						fat_disk.cachedFATsector_allocateLastOffset = 0;
				}
		}

		return FAT_ERR_FILESYSCLUSTERSFULL;
}

/* ----------------------------------------------------------------------------------------------------*/

static fat_error_t fat_cluster_getnext (const fat_cluster_t cluster, /*@notnull@*/ /*@out@*/ fat_cluster_t * const clusterNext)
{
		fat_sector_t dirSector = (fat_sector_t) (((cluster << 1) >> 9) + fat_disk.sectorsReserved);
		fat_offset_t dirOffset = (fat_offset_t) ((cluster << 1) & 0x01FF);
		fat_error_t result;

		assert (FAT_IS_CLUSTER_VALID (cluster));
		assert (clusterNext != NULL);

		if ((result = fat_cluster_sector_load (dirSector)) != FAT_ERR_NONE)
		{
				return result;
		}

		*clusterNext = (fat_cluster_t) fat_disk.cachedFATbuffer [dirOffset + 0] | ((fat_cluster_t) fat_disk.cachedFATbuffer [dirOffset + 1] << 8);

		return FAT_ERR_NONE;
}

/* ----------------------------------------------------------------------------------------------------*/

static fat_error_t fat_cluster_setnext (const fat_cluster_t cluster, const fat_cluster_t clusterNew)
{
		fat_sector_t dirSector = (fat_sector_t) (((cluster << 1) >> 9) + fat_disk.sectorsReserved);
		fat_offset_t dirOffset = (fat_offset_t) ((cluster << 1) & 0x01FF);
		fat_error_t result;

		assert (FAT_IS_CLUSTER_VALID (cluster));
		assert (FAT_IS_CLUSTER_VALID (clusterNew));

		if ((result = fat_cluster_sector_load (dirSector)) != FAT_ERR_NONE)
		{
				return result;
		}

		fat_disk.cachedFATbuffer [dirOffset + 0] = (unsigned char) ((clusterNew) & 0xFF);
		fat_disk.cachedFATbuffer [dirOffset + 1] = (unsigned char) ((clusterNew >> 8) & 0xFF);

		return fat_cluster_sector_save ();
}

/* ----------------------------------------------------------------------------------------------------*/

static fat_error_t fat_cluster_clear (const fat_cluster_t cluster)
{
		fat_sector_t dirSector = (fat_sector_t) (((cluster << 1) >> 9) + fat_disk.sectorsReserved);
		fat_offset_t dirOffset = (fat_offset_t) ((cluster << 1) & 0x01FF);
		fat_error_t result;

		assert (FAT_IS_CLUSTER_VALID (cluster));

		if ((result = fat_cluster_sector_load (dirSector)) != FAT_ERR_NONE)
		{
				return result;
		}

		fat_disk.cachedFATbuffer [dirOffset + 0] = (unsigned char) 0x00;
		fat_disk.cachedFATbuffer [dirOffset + 1] = (unsigned char) 0x00;

		return fat_cluster_sector_save ();
}

/* ----------------------------------------------------------------------------------------------------*/

static fat_error_t fat_file_sector_load (const fat_sector_t sector)
{
		assert (FAT_IS_DIR_SECTOR_VALID (sector));

		if (fat_disk.cachedDIRsector != sector)
		{
				fat_disk.cachedDIRsector = sector;
				if (card_sector_read (fat_disk.sectorsOffset + fat_disk.cachedDIRsector, fat_disk.cachedDIRbuffer) != (unsigned int) CARD_SECTOR_SIZE)
				{
						return FAT_ERR_CARDREADFAILED;
				}
		}

		return FAT_ERR_NONE;
}

static fat_error_t fat_file_sector_save (void)
{
		if (card_sector_write (fat_disk.sectorsOffset + fat_disk.cachedDIRsector, fat_disk.cachedDIRbuffer)
						!= (unsigned int) CARD_SECTOR_SIZE)
		{
				return FAT_ERR_CARDWRITEFAILED;
		}

		return FAT_ERR_NONE;
}

/* ----------------------------------------------------------------------------------------------------*/

static boolean fat_file_name_match (/*@notnull@*/ const char * const filename, /*@notnull@*/ const unsigned char * const buffer)
{
		unsigned int bufferOffset;
		unsigned int filenameOffset;

		assert (filename != NULL);
		assert (buffer != NULL);

		for (bufferOffset = (unsigned int) 0, filenameOffset = (unsigned int) 0; bufferOffset < (unsigned int) 11; bufferOffset++)
		{
				if (bufferOffset == (unsigned int) 8 && filename [filenameOffset] != '\0') /* skip '.' */
				{
						filenameOffset++;
				}

				if (filename [filenameOffset] != '\0' && (bufferOffset >= (unsigned int) 8 || filename [filenameOffset] != '.'))
				{
						/* look for dot if < 8 */
						if (UTIL_TOUPPER ((char) (buffer [bufferOffset])) != UTIL_TOUPPER (filename [filenameOffset]))
						{
								return FALSE;
						}

						filenameOffset++;
				}
				else if (buffer [bufferOffset] != (unsigned char) ' ')
				{
						return FALSE;
				}
		}

		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

static void fat_file_name_encode (/*@notnull@*/ const char * const filename, /*@notnull@*/ /*@out@*/ unsigned char * const buffer)
{
		unsigned int bufferOffset;
		unsigned int filenameOffset;

		assert (filename != NULL);
		assert (buffer != NULL);

		for (bufferOffset = (unsigned int) 0, filenameOffset = (unsigned int) 0; bufferOffset < (unsigned int) 11; bufferOffset++)
		{
				if (bufferOffset == (unsigned int) 8 && filename [filenameOffset] != '\0') /* skip '.' */
				{
						filenameOffset++;
				}

				if (filename [filenameOffset] != '\0' && (bufferOffset >= (unsigned int) 8 || filename [filenameOffset] != '.'))
				{
						buffer [bufferOffset] = (unsigned char) UTIL_TOUPPER (filename [filenameOffset]);

						filenameOffset++;
				}
				else
				{
						buffer [bufferOffset] = (unsigned char) 0x20;
				}
		}
}

/* ----------------------------------------------------------------------------------------------------*/

static void fat_file_name_decode (/*@notnull@*/ const unsigned char * const buffer, /*@notnull@*/ /*@out@*/ char * const filename)
{
		unsigned int bufferOffset;
		unsigned int filenameOffset = (unsigned int) 0;

		assert (buffer != NULL);
		assert (filename != NULL);

		for (bufferOffset = (unsigned int) 0; bufferOffset < (unsigned int) 8 && buffer [bufferOffset] != (unsigned char) 0x20; bufferOffset++)
		{
				filename [filenameOffset++] = (char) buffer [bufferOffset];
		}
		if (buffer [8] != (unsigned char) 0x20)
		{
				filename [filenameOffset++] = '.';
		}
		for (bufferOffset = (unsigned int) 8; bufferOffset < (unsigned int) 11 && buffer [bufferOffset] != (unsigned char) 0x20; bufferOffset++)
		{
				filename [filenameOffset++] = (char) buffer [bufferOffset];
		}

		filename [filenameOffset++] = '\0';
}

/* ----------------------------------------------------------------------------------------------------*/

static fat_error_t fat_file_name_check (/*@notnull@*/ const char * const filename)
{
		unsigned int fileSize;
		unsigned int filenameOffset;

		assert (filename != NULL);

		for (filenameOffset = (unsigned int) 0, fileSize = (unsigned int) 0; 
						filename [filenameOffset] != '\0' && filename [filenameOffset] != '.'; filenameOffset++, fileSize++)
		{
				if (filename [filenameOffset] == (char) 0x5C)
				{
						return FAT_ERR_FILENAMENOTSUPPORTED;
				}

				if (fileSize > (unsigned int) 8)
				{
						return FAT_ERR_FILENAMETOOLONG;
				}
		}

		return FAT_ERR_NONE;
}

/* ----------------------------------------------------------------------------------------------------*/

static fat_error_t fat_file_entry_allocate (/*@notnull@*/ /*@out@*/ fat_sector_t * const dirSector, /*@notnull@*/ /*@out@*/ fat_offset_t * const dirOffset)
{
		fat_offset_t entryOffset;
		fat_offset_t sectorOffset;
		fat_error_t result;

		assert (dirSector != NULL);
		assert (dirOffset != NULL);

		for (sectorOffset = 0; sectorOffset < fat_disk.rootSectors; sectorOffset++)
		{
				if ((result = fat_file_sector_load (fat_disk.rootDirectory + sectorOffset)) != FAT_ERR_NONE)
				{
						return result;
				}

				for (entryOffset = 0; entryOffset < (fat_offset_t) CARD_SECTOR_SIZE; entryOffset += (fat_offset_t) 32)
				{
						if (((fat_disk.cachedDIRbuffer [entryOffset] & 0xFF) == (unsigned char) 0x00) ||
										((fat_disk.cachedDIRbuffer [entryOffset] & 0xFF) == (unsigned char) 0xE5))
						{
								*dirSector = fat_disk.rootDirectory + sectorOffset;
								*dirOffset = entryOffset;

								return FAT_ERR_NONE;
						}
				}
		}

		return FAT_ERR_FILESYSDIRECTORYFULL;
}

/* ----------------------------------------------------------------------------------------------------*/

static fat_error_t fat_file_entry_locate (/*@notnull@*/ const char * const filename, /*@notnull@*/ /*@out@*/ fat_sector_t * const dirSector, /*@notnull@*/ /*@out@*/ fat_offset_t * const dirOffset)
{
		fat_offset_t entryOffset;
		fat_offset_t sectorOffset;
		fat_error_t result;

		assert (filename != NULL);
		assert (dirSector != NULL);
		assert (dirOffset != NULL);

		for (sectorOffset = 0; sectorOffset < fat_disk.rootSectors; sectorOffset++)
		{
				if ((result = fat_file_sector_load (fat_disk.rootDirectory + sectorOffset)) != FAT_ERR_NONE)
				{
						return result;
				}

				for (entryOffset = 0; entryOffset < (fat_offset_t) CARD_SECTOR_SIZE; entryOffset += (fat_offset_t) 32)
				{
						if (fat_disk.cachedDIRbuffer [entryOffset] == (unsigned char) '\0') /* end of directory */
						{
								return FAT_ERR_FILENOTFOUND;
						}

						if (((fat_disk.cachedDIRbuffer [entryOffset + 11] & 0xDE) == (unsigned char) 0x00) && 
							((fat_disk.cachedDIRbuffer [entryOffset] & 0xFF) != (unsigned char) 0xE5))
						{
								if (fat_file_name_match (filename, &fat_disk.cachedDIRbuffer [entryOffset]))
								{
										*dirSector = fat_disk.rootDirectory + sectorOffset;
										*dirOffset = entryOffset;

										return FAT_ERR_NONE;
								}
						}
				}
		}

		return FAT_ERR_FILENOTFOUND;
}

/* ----------------------------------------------------------------------------------------------------*/

static fat_error_t fat_file_locate (/*@notnull@*/ const char * const filename, /*@notnull@*/ /*@out@*/ fat_sector_t * const dirSector, /*@notnull@*/ /*@out@*/ fat_offset_t * const dirOffset, /*@notnull@*/ /*@out@*/ fat_cluster_t * const fileCluster, /*@notnull@*/ /*@out@*/ fat_filesize_t * const fileSize)
{
		fat_error_t result;

		assert (filename != NULL);
		assert (dirSector != NULL);
		assert (dirOffset != NULL);

		if ((result = fat_file_entry_locate (filename, dirSector, dirOffset)) != FAT_ERR_NONE)
		{
				return result;
		}

		*fileCluster = (fat_cluster_t) fat_disk.cachedDIRbuffer [(*dirOffset) + 26] | ((fat_cluster_t) fat_disk.cachedDIRbuffer [(*dirOffset) + 27] << 8);
		*fileSize = (fat_filesize_t) fat_disk.cachedDIRbuffer [(*dirOffset) + 28] + ((fat_filesize_t) fat_disk.cachedDIRbuffer [(*dirOffset) + 29] << 8) +
					((fat_filesize_t) fat_disk.cachedDIRbuffer [(*dirOffset) + 30] << 16) + ((fat_filesize_t) fat_disk.cachedDIRbuffer [(*dirOffset) + 31] << 24);

		return FAT_ERR_NONE;
}

/* ----------------------------------------------------------------------------------------------------*/

/*@notnull@*/ /*@shared@*/ static const unsigned char * fat_file_clock_buffer (void)
{
		static unsigned char buffer [5];
		util_clock_buf_t cb;

		(void) util_clock_get (cb);

		buffer [0] = (unsigned char) ((cb [0] & 0x01) * (unsigned char) 100);
		buffer [1] = (unsigned char) (((cb [0] >> 1) & 0x1F) | (cb [1] << 5));
		buffer [2] = (unsigned char) (((cb [1] >> 3) & 0x07) | (cb [2] << 3));
		buffer [3] = (unsigned char) ((cb [3] & 0x1F) | ((cb [4] << 5) & 0xE0));
		buffer [4] = (unsigned char) (((cb [5] >> 3) & 0x01) | (((cb [6] & 0xFF) << 1) & 0xFE));

		return buffer;
}

/* ----------------------------------------------------------------------------------------------------*/

static fat_error_t fat_file_create (/*@notnull@*/ const char * const filename, /*@notnull@*/ /*@out@*/ fat_sector_t * const dirSector, /*@notnull@*/ /*@out@*/ fat_offset_t * const dirOffset, /*@notnull@*/ /*@out@*/ fat_cluster_t * const fileCluster)
{
		const unsigned char * clockBuffer;
		unsigned char * directoryBuffer;
		fat_error_t result;

		assert (filename != NULL);
		assert (dirSector != NULL);
		assert (dirOffset != NULL);
		assert (fileCluster != NULL);

		if ((result = fat_file_name_check (filename)) != FAT_ERR_NONE)
		{
				return result;
		}

		if ((result = fat_file_entry_allocate (dirSector, dirOffset)) != FAT_ERR_NONE)
		{
				return result;
		}

		(*fileCluster) = 0x0000;

		clockBuffer = fat_file_clock_buffer ();

		directoryBuffer = &fat_disk.cachedDIRbuffer [(*dirOffset)];

		fat_file_name_encode (filename, &directoryBuffer [0]);

		directoryBuffer [11] = (unsigned char) 0x20;

		directoryBuffer [12] = (unsigned char) 0x00;

		directoryBuffer [13] = clockBuffer [0];
		directoryBuffer [14] = clockBuffer [1];
		directoryBuffer [15] = clockBuffer [2];
		directoryBuffer [16] = clockBuffer [3];
		directoryBuffer [17] = clockBuffer [4];

		directoryBuffer [18] = clockBuffer [3];
		directoryBuffer [19] = clockBuffer [4];

		directoryBuffer [20] = (unsigned char) 0x00;
		directoryBuffer [21] = (unsigned char) 0x00;

		directoryBuffer [22] = clockBuffer [1];
		directoryBuffer [23] = clockBuffer [2];
		directoryBuffer [24] = clockBuffer [3];
		directoryBuffer [25] = clockBuffer [4];

		directoryBuffer [26] = (unsigned char) ((*fileCluster) & 0xFF);
		directoryBuffer [27] = (unsigned char) (((*fileCluster) >> 8) & 0xFF);

		directoryBuffer [28] = (unsigned char) 0x00;
		directoryBuffer [29] = (unsigned char) 0x00;
		directoryBuffer [30] = (unsigned char) 0x00;
		directoryBuffer [31] = (unsigned char) 0x00;

		return fat_file_sector_save ();
}

/* ----------------------------------------------------------------------------------------------------*/

static fat_error_t fat_file_update_write (const fat_sector_t dirSector, const fat_offset_t dirOffset, const fat_cluster_t fileCluster, const fat_filesize_t fileSize)
{
		const unsigned char * const clockBuffer = fat_file_clock_buffer ();
		unsigned char * directoryBuffer;
		fat_error_t result;

		assert (FAT_IS_DIR_SECTOR_VALID (dirSector));
		assert (FAT_IS_DIR_OFFSET_VALID (dirOffset));

		if ((result = fat_file_sector_load (dirSector)) != FAT_ERR_NONE)
		{
				return result;
		}

		directoryBuffer = &fat_disk.cachedDIRbuffer [dirOffset];

		directoryBuffer [18] = clockBuffer [3];
		directoryBuffer [19] = clockBuffer [4];

		directoryBuffer [22] = clockBuffer [1];
		directoryBuffer [23] = clockBuffer [2];
		directoryBuffer [24] = clockBuffer [3];
		directoryBuffer [25] = clockBuffer [4];

		directoryBuffer [26] = (unsigned char) ((fileCluster) & 0xFF);
		directoryBuffer [27] = (unsigned char) (((fileCluster) >> 8) & 0xFF);

		directoryBuffer [28] = (unsigned char) ((fileSize) & 0xFF);
		directoryBuffer [29] = (unsigned char) ((fileSize >> 8) & 0xFF);
		directoryBuffer [30] = (unsigned char) ((fileSize >> 16) & 0xFF);
		directoryBuffer [31] = (unsigned char) ((fileSize >> 24) & 0xFF);

		return fat_file_sector_save ();
}

/* ----------------------------------------------------------------------------------------------------*/

static fat_error_t fat_file_update_read (const fat_sector_t dirSector, const fat_offset_t dirOffset)
{
		const unsigned char * const clockBuffer = fat_file_clock_buffer ();
		unsigned char * directoryBuffer;
		fat_error_t result;

		assert (FAT_IS_DIR_SECTOR_VALID (dirSector));
		assert (FAT_IS_DIR_OFFSET_VALID (dirOffset));

		if ((result = fat_file_sector_load (dirSector)) != FAT_ERR_NONE)
		{
				return result;
		}

		directoryBuffer = &fat_disk.cachedDIRbuffer [dirOffset];

		directoryBuffer [18] = clockBuffer [3];
		directoryBuffer [19] = clockBuffer [4];

		return fat_file_sector_save ();
}

/* ----------------------------------------------------------------------------------------------------*/

static fat_error_t fat_file_unlink (const fat_sector_t dirSector, const fat_offset_t dirOffset, const fat_cluster_t fileCluster)
{
		unsigned char * directoryBuffer;
		fat_cluster_t cluster;
		fat_error_t result;

		assert (FAT_IS_DIR_SECTOR_VALID (dirSector));
		assert (FAT_IS_DIR_OFFSET_VALID (dirOffset));

		if ((result = fat_file_sector_load (dirSector)) != FAT_ERR_NONE)
		{
				return result;
		}

		directoryBuffer = &fat_disk.cachedDIRbuffer [dirOffset];

		directoryBuffer [0] = (unsigned char) 0xE5;

		if ((result = fat_file_sector_save ()) != FAT_ERR_NONE)
		{
				return result;
		}

		cluster = fileCluster;
		while (fat_cluster_valid (cluster))
		{
				fat_cluster_t clusterNext;

				if ((result = fat_cluster_getnext (cluster, &clusterNext)) != FAT_ERR_NONE)
				{
						return result;
				}

				if ((result = fat_cluster_clear (cluster)) != FAT_ERR_NONE)
				{
						return result;
				}

				cluster = clusterNext;
		}

		return FAT_ERR_NONE;
}

/* ----------------------------------------------------------------------------------------------------*/

static void fat_handle_init (void)
{
		fat_handle_t handle;

		for (handle = (fat_handle_t) 0; handle < FAT_FILE_SIZE; handle += (fat_handle_t) 1)
		{
				fat_file [handle].options = FAT_OPTION_NONE;
		}
}

/* ----------------------------------------------------------------------------------------------------*/

/*@null@*/ /*@shared@*/ static fat_file_t * fat_handle_allocate (void)
{
		fat_handle_t handle;

		for (handle = (fat_handle_t) 0; handle < FAT_FILE_SIZE; handle += (fat_handle_t) 1)
		{
				if ((fat_file [handle].options & FAT_OPTION_ACTIVE) == FAT_OPTION_NONE)
				{
						fat_file [handle].handle = handle;
						/*@i@*/ return &fat_file [handle];
				}
		}

		return NULL;
}

/* ----------------------------------------------------------------------------------------------------*/

/*@null@*/ /*@shared@*/ static fat_file_t * fat_handle_locate (const fat_handle_t handle)
{
		if (handle >= (fat_handle_t) 0 && handle < FAT_FILE_SIZE && (fat_file [handle].options & FAT_OPTION_ACTIVE) != FAT_OPTION_NONE)
		{
				/*@i@*/ return &fat_file [handle];
		}

		return NULL;
}

/* ----------------------------------------------------------------------------------------------------*/

static fat_handle_t fat_handle_setup (/*@notnull@*/ /*@shared@*/ fat_file_t * const fileHandle, const fat_sector_t dirSector, const fat_offset_t dirOffset, const fat_cluster_t fileCluster, const fat_filesize_t fileSize, const unsigned char options)
{
		assert (fileHandle != NULL);
		assert (FAT_IS_DIR_SECTOR_VALID (dirSector));
		assert (FAT_IS_DIR_OFFSET_VALID (dirOffset));

		fileHandle->dirSector = dirSector;
		fileHandle->dirOffset = dirOffset;
		fileHandle->fileCluster = fileCluster;
		fileHandle->fileSize = fileSize;
		fileHandle->currentCluster = fileCluster;
		fileHandle->currentSector = 0;
		fileHandle->currentOffset = 0;
		fileHandle->options = options | FAT_OPTION_ACTIVE;
		fileHandle->status = FAT_STATUS_NONE;

		return fileHandle->handle;
}

/* ----------------------------------------------------------------------------------------------------*/

static fat_error_t fat_file_sync (/*@notnull@*/ fat_file_t * const fileHandle)
{
		fat_error_t result;

		if ((fileHandle->status & FAT_STATUS_UNSYNC) != FAT_STATUS_NONE)
		{
				if ((fileHandle->options & FAT_OPTION_READ) != FAT_OPTION_NONE)
				{
						result = fat_file_update_read (fileHandle->dirSector, fileHandle->dirOffset);
				}
				else
				{
						result = fat_file_update_write (fileHandle->dirSector, fileHandle->dirOffset, fileHandle->fileCluster, fileHandle->fileSize);
				}

				fileHandle->status &= ~FAT_STATUS_UNSYNC;

				return result;
		}

		return FAT_ERR_NONE;
}

/* ----------------------------------------------------------------------------------------------------*/

static fat_error_t fat_file_cache_sector_load (/*@notnull@*/ fat_file_t * const fileHandle, const fat_sector_t sector);

static fat_error_t fat_file_seek (/*@notnull@*/ fat_file_t * const fileHandle, fat_seek_t seek)
{
		fat_error_t result;
		fat_filesize_t offset;

		assert (fileHandle != NULL);
		assert (seek == FAT_SEEK_END);
		assert (fileHandle->currentOffset == 0); /* XXX currently */

		FAT_EMULATE_HOOK_FILE_SEEK (fileHandle->handle, seek);

		if (fileHandle->fileSize == 0)
		{
				return FAT_ERR_NONE;
		}

		for (offset = 0; (offset + CARD_SECTOR_SIZE) < fileHandle->fileSize; offset += CARD_SECTOR_SIZE)
		{
				if (!fat_cluster_valid (fileHandle->currentCluster))
				{
						return FAT_ERR_FILEINCONSISTENCY;
				}

				if (++(fileHandle->currentSector) == (fat_sector_t) fat_disk.sectorsPerCluster)
				{
						fileHandle->currentSector = 0;
				}

				if (fileHandle->currentSector == 0)
				{
						if ((result = fat_cluster_getnext (fileHandle->currentCluster, &fileHandle->currentCluster)) != FAT_ERR_NONE)
						{
								return result;
						}
				}
		}

		assert ((fileHandle->fileSize - offset) <= (unsigned long) CARD_SECTOR_SIZE);

		if ((fileHandle->fileSize - offset) < (unsigned long) CARD_SECTOR_SIZE)
		{
				if ((result = fat_file_cache_sector_load (fileHandle, fat_cluster_to_sector (fileHandle->currentCluster) + fileHandle->currentSector)) != FAT_ERR_NONE)
				{
						return result;
				}

				fileHandle->currentOffset = (unsigned int) (fileHandle->fileSize - offset);
		}
		else
		{
				if (fileHandle->currentSector == (fat_sector_t) fat_disk.sectorsPerCluster)
				{
						fat_cluster_t cluster = 0;

						if ((result = fat_cluster_allocate (&cluster)) != FAT_ERR_NONE)
						{
								return result;
						}

						if ((result = fat_cluster_setnext (fileHandle->currentCluster, cluster)) != FAT_ERR_NONE)
						{
								return result;
						}

						fileHandle->currentCluster = cluster;
						fileHandle->currentSector = 0;
				}
				else
				{
						fileHandle->currentSector++;
				}
		}

		return FAT_ERR_NONE;
}

/* ----------------------------------------------------------------------------------------------------*/

static fat_error_t fat_handle_checkifinuse (const fat_cluster_t cluster)
{
		fat_handle_t handle;

		if (fat_cluster_valid (cluster))
		{
				for (handle = (fat_handle_t) 0; handle < FAT_FILE_SIZE; handle += (fat_handle_t) 1)
				{
						if ((fat_file [handle].options & FAT_OPTION_ACTIVE) != FAT_OPTION_NONE &&
										fat_cluster_valid (fat_file [handle].fileCluster) && fat_file [handle].fileCluster == cluster)
						{
								return (fat_file [handle].options & FAT_OPTION_READ) != FAT_OPTION_NONE ? FAT_ERR_FILEOPENFORREADALREADY : FAT_ERR_FILEOPENFORWRITEALREADY;
						}
				}
		}

		return FAT_ERR_NONE;
}

/* ----------------------------------------------------------------------------------------------------*/

fat_error_t fat_init (const fat_config_t * const config)
{
		unsigned char buffer [CARD_SECTOR_SIZE];
		unsigned char partinfo [FAT_PARTINFO_LENGTH];
		fat_error_t result;

		assert (config != NULL);

		FAT_EMULATE_HOOK_INIT ();

		FAT_CHECK_FATCONFIG (config);

		fat_conf = FAT_CONFIG_NONE;

		if (card_init () == FALSE)
		{
				return FAT_ERR_CARDINITFAILED;
		}

		if (card_enable () == FALSE)
		{
				return FAT_ERR_CARDENABLEFAILED;
		}

		if (card_config_blocklength () == FALSE)
		{
				return FAT_ERR_CARDCONFIGFAILED;
		}

		fat_disk.fileSys = FAT_FILESYS_UNKNOWN;

		if (fat_filesys_load (buffer, partinfo) != FAT_ERR_NONE || fat_filesys_identify (partinfo) != FAT_ERR_NONE)
		{
				if ((result = fat_filesys_getpartoffset (buffer, &fat_disk.sectorsOffset)) != FAT_ERR_NONE)
				{
						return result;
				}

				if ((result = fat_filesys_load (buffer, partinfo)) != FAT_ERR_NONE || (result = fat_filesys_identify (partinfo)) != FAT_ERR_NONE)
				{
						return result;
				}
		}

		if ((result = fat_filesys_getinfo (partinfo)) != FAT_ERR_NONE)
		{
				return result;
		}

		fat_handle_init ();

		fat_cluster_sector_init ();

		DPRINTF (("fat_init: filesys=%d\n", fat_disk.fileSys));

		return FAT_ERR_NONE;
}

/* ----------------------------------------------------------------------------------------------------*/

fat_error_t fat_term (void)
{
		fat_handle_t handle;

		FAT_EMULATE_HOOK_TERM ();

		FAT_CHECK_CARD;
		FAT_CHECK_FILESYS;

		DPRINTF (("fat_term\n"));

		for (handle = (fat_handle_t) 0; handle < FAT_FILE_SIZE; handle += (fat_handle_t) 1)
		{
				if ((fat_file [handle].options & FAT_OPTION_ACTIVE) != FAT_OPTION_NONE)
				{
						DPRINTF (("fat_term: error, handle %d still active - closing!\n", handle));
						(void) fat_close (handle); /* ignore error! */
				}
		}

		(void) fat_flush (); /* ignore error! */

		fat_cluster_sector_term ();

		(void) card_term (); /* ignore error! */

		return FAT_ERR_NONE;
}

/* ----------------------------------------------------------------------------------------------------*/

fat_error_t fat_suspend (void)
{
		FAT_EMULATE_HOOK_SUSPEND ();

		if (card_state_suspend () == FALSE)
		{
				return FAT_ERR_CARDSUSPENDFAILED;
		}

		return FAT_ERR_NONE;
}

/* ----------------------------------------------------------------------------------------------------*/

fat_error_t fat_resume (void)
{
		FAT_EMULATE_HOOK_RESUME ();

		if (card_state_resume () == FALSE)
		{
				return FAT_ERR_CARDRESUMEFAILED;
		}

		return FAT_ERR_NONE;
}

/* ----------------------------------------------------------------------------------------------------*/

fat_error_t fat_dirent_init (fat_dirent_t* const dirent)
{
		assert (dirent != NULL);

		FAT_EMULATE_HOOK_DIRENT_INIT (dirent);

		(void) util_memset ((unsigned char *) dirent, (unsigned char) 0, (unsigned int) sizeof (fat_dirent_t));

		return FAT_ERR_NONE;
}

fat_error_t fat_dirent_next (fat_dirent_t* const dirent)
{
		fat_offset_t sectorOffset;
		fat_offset_t entryOffset;
		fat_error_t result;

		assert (dirent != NULL);

		FAT_EMULATE_HOOK_DIRENT_NEXT (dirent);

		sectorOffset = (fat_offset_t) ((dirent->state >> 16) & 0xFFFF);
		entryOffset = (fat_offset_t) (dirent->state & 0xFFFF);

		while (sectorOffset < fat_disk.rootSectors)
		{
				if ((result = fat_file_sector_load (fat_disk.rootDirectory + sectorOffset)) != FAT_ERR_NONE)
				{
						return result;
				}

				while (entryOffset < (fat_offset_t) CARD_SECTOR_SIZE)
				{
						if (fat_disk.cachedDIRbuffer [entryOffset] == (unsigned char) '\0') /* end of directory */
						{
								return FAT_ERR_ENDOFDIRECTORY;
						}

						if (((fat_disk.cachedDIRbuffer [entryOffset + 11] & 0xDE) == (unsigned char) 0x00) && 
							((fat_disk.cachedDIRbuffer [entryOffset] & 0xFF) != (unsigned char) 0xE5))
						{
								fat_file_name_decode (&fat_disk.cachedDIRbuffer [entryOffset], dirent->name);

								dirent->size = (fat_filesize_t) fat_disk.cachedDIRbuffer [entryOffset + 28] + 
										   ((fat_filesize_t) fat_disk.cachedDIRbuffer [entryOffset + 29] << 8) +
										   ((fat_filesize_t) fat_disk.cachedDIRbuffer [entryOffset + 30] << 16) +
										   ((fat_filesize_t) fat_disk.cachedDIRbuffer [entryOffset + 31] << 24);

								dirent->state = (unsigned long) ((sectorOffset << 16) | (entryOffset + 32));

								return FAT_ERR_NONE;
						}

						entryOffset += (fat_offset_t) 32;
				}

				entryOffset = (fat_offset_t) 0;
				sectorOffset++;
		}

		return FAT_ERR_ENDOFDIRECTORY;
}

/* ----------------------------------------------------------------------------------------------------*/

fat_handle_t fat_open (const char * const filename, const fat_options_t fileopts)
{
		fat_file_t * fileHandle;
		fat_sector_t dirSector = 0;
		fat_offset_t dirOffset = 0;
		fat_cluster_t fileCluster = 0;
		fat_filesize_t fileSize = 0;
		unsigned char options = FAT_OPTION_NONE;
		fat_error_t result;

		FAT_EMULATE_HOOK_OPEN (filename, fileopts);

		fileHandle = fat_handle_allocate ();

		FAT_CHECK_CARD;
		FAT_CHECK_FILESYS;
		FAT_CHECK_HANDLE (fileHandle);
		FAT_CHECK_FILENAME (filename);
		FAT_CHECK_FATOPTIONS (fileopts);

		if ((fileopts & (FAT_OPT_READ|FAT_OPT_WRITE)) == FAT_OPT_NONE || ((fileopts & (FAT_OPT_READ|FAT_OPT_WRITE)) == (fat_options_t) (FAT_OPT_READ|FAT_OPT_WRITE)))
		{
				return FAT_ERR_ACCESSMODEINVALID;
		}
		if ((fileopts & FAT_OPT_READ) != FAT_OPT_NONE && (fileopts & FAT_OPT_APPEND) != FAT_OPT_NONE)
		{
				return FAT_ERR_ACCESSMODEINVALID;
		}
		if ((fileopts & FAT_OPT_SYNC) != FAT_OPT_NONE)
		{
				options |= FAT_OPTION_SYNC;
		}

		result = fat_file_locate (filename, &dirSector, &dirOffset, &fileCluster, &fileSize);

		if ((fileopts & FAT_OPT_READ) != FAT_OPT_NONE)
		{
				if (result == FAT_ERR_NONE)
				{
						if ((result = fat_handle_checkifinuse (fileCluster)) == FAT_ERR_NONE)
						{
								return fat_handle_setup (fileHandle, dirSector, dirOffset, fileCluster, fileSize, FAT_OPTION_READ|options);
						}
				}
		}
		else if ((fileopts & FAT_OPT_WRITE) != FAT_OPT_NONE)
		{
				if (result == FAT_ERR_FILENOTFOUND)
				{
						if ((result = fat_file_create (filename, &dirSector, &dirOffset, &fileCluster)) == FAT_ERR_NONE)
						{
								return fat_handle_setup (fileHandle, dirSector, dirOffset, fileCluster, 0, FAT_OPTION_WRITE|options);
						}
				}
				else
				{
						if ((fileopts & FAT_OPT_APPEND) != FAT_OPT_NONE)
						{
								if ((result = fat_handle_setup (fileHandle, dirSector, dirOffset, fileCluster, fileSize, FAT_OPTION_WRITE|options)) == FAT_ERR_NONE)
								{
										return fat_file_seek (fileHandle, FAT_SEEK_END);
								}
						}
						else
						{
								result = FAT_ERR_FILEALREADYEXISTS;
						}
				}
		}

		return (fat_handle_t) result;
}

/* ----------------------------------------------------------------------------------------------------*/

static fat_error_t fat_file_cache_sector_load (/*@notnull@*/ fat_file_t * const fileHandle, const fat_sector_t sector)
{
		assert (fileHandle != NULL);
		assert (FAT_IS_FIL_SECTOR_VALID (sector));

		if (fileHandle->cachedSector != sector)
		{
				fileHandle->cachedSector = sector;

				if (card_sector_read (fat_disk.sectorsOffset + fileHandle->cachedSector, fileHandle->cachedBuffer) != (unsigned int) CARD_SECTOR_SIZE)
				{
						return FAT_ERR_CARDREADFAILED;
				}
		}

		return FAT_ERR_NONE;
}

static fat_error_t fat_file_cache_sector_save (/*@notnull@*/ fat_file_t * const fileHandle)
{
		assert (fileHandle != NULL);

		if (card_sector_write (fat_disk.sectorsOffset + fileHandle->cachedSector, fileHandle->cachedBuffer)
						!= (unsigned int) CARD_SECTOR_SIZE)
		{
				return FAT_ERR_CARDWRITEFAILED;
		}

		return FAT_ERR_NONE;
}

static fat_error_t fat_file_cache_sector_save_through (/*@notnull@*/ fat_file_t * const fileHandle, const fat_sector_t sector, /*@notnull@*/ const unsigned char* const buffer)
{
		assert (fileHandle != NULL);
		assert (FAT_IS_FIL_SECTOR_VALID (sector));
		assert (buffer != NULL);

		if (card_sector_write (fat_disk.sectorsOffset + sector, buffer)
						!= (unsigned int) CARD_SECTOR_SIZE)
		{
				return FAT_ERR_CARDWRITEFAILED;
		}

		fileHandle->cachedSector = 0; /* force it to be flushed to avoid inconsistency (unlikely ...) */

		return FAT_ERR_NONE;
}

static fat_error_t fat_file_cache_sector_load_through (/*@notnull@*/ fat_file_t * const fileHandle, const fat_sector_t sector, /*@notnull@*/ /*@out@*/ unsigned char* const buffer)
{
		assert (fileHandle != NULL);
		assert (FAT_IS_FIL_SECTOR_VALID (sector));
		assert (buffer != NULL);

		if (card_sector_read (fat_disk.sectorsOffset + sector, buffer) != (unsigned int) CARD_SECTOR_SIZE)
		{
				return FAT_ERR_CARDREADFAILED;
		}

		fileHandle->cachedSector = 0; /* force it to be flushed to avoid inconsistency (unlikely ...) */

		return FAT_ERR_NONE;
}

/* ----------------------------------------------------------------------------------------------------*/

fat_error_t fat_read (fat_handle_t handle, unsigned char * const buffer, const unsigned int length, unsigned int * const actual)
{
		fat_file_t * fileHandle;
		fat_error_t result;
		unsigned int length2;
		unsigned int offset;

		FAT_EMULATE_HOOK_READ (handle, buffer, length, actual);

		fileHandle = fat_handle_locate (handle);

		FAT_CHECK_CARD;
		FAT_CHECK_FILESYS;
		FAT_CHECK_HANDLE (fileHandle);
		FAT_CHECK_MODE (fileHandle->options, FAT_OPTION_READ);
		FAT_CHECK_TRANSFER_DATA (buffer);
		FAT_CHECK_TRANSFER_SIZE (length);

		length2 = (unsigned int) (UTIL_MIN ((fat_filesize_t) length, fileHandle->fileSize));
		offset = 0;

		while (offset < length2 && fat_cluster_valid (fileHandle->currentCluster))
		{
				fat_sector_t sectorToRead = fat_cluster_to_sector (fileHandle->currentCluster) + fileHandle->currentSector;
				unsigned int bytesToRead = UTIL_MIN ((unsigned int) (CARD_SECTOR_SIZE - fileHandle->currentOffset), length2 - offset);

				if (fileHandle->currentOffset == 0 && bytesToRead == (unsigned int) CARD_SECTOR_SIZE)
				{
						if ((result = fat_file_cache_sector_load_through (fileHandle, sectorToRead, &buffer [offset])) != FAT_ERR_NONE)
						{
								return result;
						}
				}
				else
				{
						if ((result = fat_file_cache_sector_load (fileHandle, sectorToRead)) != FAT_ERR_NONE)
						{
								return result;
						}

						(void) util_memcpy (&buffer [offset], &(fileHandle->cachedBuffer [fileHandle->currentOffset]), bytesToRead);

						fileHandle->currentOffset = (fileHandle->currentOffset + bytesToRead) % CARD_SECTOR_SIZE;
				}

				offset += bytesToRead;

				if (fileHandle->currentOffset == 0)
				{
						if (++(fileHandle->currentSector) == (fat_sector_t) fat_disk.sectorsPerCluster)
						{
								fileHandle->currentSector = 0;
						}

						if (fileHandle->currentSector == 0)
						{
								if ((result = fat_cluster_getnext (fileHandle->currentCluster, &fileHandle->currentCluster)) != FAT_ERR_NONE)
								{
										return result;
								}
						}
				}
		}

		if (actual != NULL)
		{
				*actual = length2;
		}

		fileHandle->fileSize -= length2;

		fileHandle->status |= FAT_STATUS_UNSYNC;

		if ((fileHandle->options & FAT_OPTION_SYNC) != FAT_OPTION_NONE)
		{
				if ((result = fat_file_sync (fileHandle)) != FAT_ERR_NONE)
				{
						return result;
				}
		}

		return FAT_ERR_NONE;
}

/* ----------------------------------------------------------------------------------------------------*/

fat_error_t fat_write (fat_handle_t handle, const unsigned char * const buffer, const unsigned int length, unsigned int * const actual)
{
		fat_file_t * fileHandle;
		fat_error_t result;
		unsigned int offset = 0;

		FAT_EMULATE_HOOK_WRITE (handle, buffer, length, actual);

		fileHandle = fat_handle_locate (handle);

		FAT_CHECK_CARD;
		FAT_CHECK_FILESYS;
		FAT_CHECK_HANDLE (fileHandle);
		FAT_CHECK_MODE (fileHandle->options, FAT_OPTION_WRITE);
		FAT_CHECK_TRANSFER_DATA (buffer);
		FAT_CHECK_TRANSFER_SIZE (length);

		while (offset < length)
		{
				fat_sector_t sectorToWrite;
				unsigned int bytesToWrite;

				if (fat_cluster_valid (fileHandle->currentCluster) == FALSE || fileHandle->currentSector == (fat_sector_t) fat_disk.sectorsPerCluster)
				{
						fat_cluster_t cluster = 0;

						if ((result = fat_cluster_allocate (&cluster)) != FAT_ERR_NONE)
						{
								return result;
						}

						if (fat_cluster_valid (fileHandle->currentCluster) == FALSE)
						{
								fileHandle->fileCluster = cluster;
						}
						else
						{
								if ((result = fat_cluster_setnext (fileHandle->currentCluster, cluster)) != FAT_ERR_NONE)
								{
										return result;
								}
						}

						fileHandle->currentCluster = cluster;
						fileHandle->currentSector = 0;
				}

				sectorToWrite = fat_cluster_to_sector (fileHandle->currentCluster) + fileHandle->currentSector;
				bytesToWrite = UTIL_MIN ((unsigned int) (CARD_SECTOR_SIZE - fileHandle->currentOffset), length - offset);

				if (fileHandle->currentOffset == 0 && bytesToWrite == (unsigned int) CARD_SECTOR_SIZE)
				{
						if ((result = fat_file_cache_sector_save_through (fileHandle, sectorToWrite, &buffer [offset])) != FAT_ERR_NONE)
						{
								return result;
						}
				}
				else
				{
						if ((result = fat_file_cache_sector_load (fileHandle, sectorToWrite)) != FAT_ERR_NONE)
						{
								return result;
						}

						(void) util_memcpy (&(fileHandle->cachedBuffer [fileHandle->currentOffset]), &buffer [offset], bytesToWrite);

						if ((result = fat_file_cache_sector_save (fileHandle)) != FAT_ERR_NONE)
						{
								return result;
						}

						fileHandle->currentOffset = (fileHandle->currentOffset + bytesToWrite) % CARD_SECTOR_SIZE;
				}

				offset += bytesToWrite;

				if (fileHandle->currentOffset == 0)
				{
						fileHandle->currentSector++;
				}
		}

		if (actual != NULL)
		{
				*actual = length;
		}

		fileHandle->fileSize += length;

		fileHandle->status |= FAT_STATUS_UNSYNC;

		if ((fileHandle->options & FAT_OPTION_SYNC) != FAT_OPTION_NONE)
		{
				if ((result = fat_file_sync (fileHandle)) != FAT_ERR_NONE)
				{
						return result;
				}
		}

		return FAT_ERR_NONE;
}

/* ----------------------------------------------------------------------------------------------------*/

fat_error_t fat_close (fat_handle_t handle)
{
		fat_file_t * fileHandle;
		fat_error_t result;

		FAT_EMULATE_HOOK_CLOSE (handle);

		fileHandle = fat_handle_locate (handle);

		FAT_CHECK_CARD;
		FAT_CHECK_FILESYS;
		FAT_CHECK_HANDLE (fileHandle);

		result = fat_file_sync (fileHandle);

		fileHandle->options = FAT_OPTION_NONE;

		if (result == FAT_ERR_NONE)
		{
				result = fat_cluster_sector_sync ();
		}

		return result;
}

/* ----------------------------------------------------------------------------------------------------*/

fat_error_t fat_tell (fat_handle_t handle, fat_filesize_t * const filesize)
{
		fat_file_t * fileHandle;

		FAT_EMULATE_HOOK_TELL (handle, filesize);

		fileHandle = fat_handle_locate (handle);

		FAT_CHECK_CARD;
		FAT_CHECK_FILESYS;
		FAT_CHECK_HANDLE (fileHandle);

		assert (filesize != NULL);

		if ((fileHandle->options & FAT_OPTION_READ) != FAT_OPTION_NONE)
		{
				return FAT_ERR_NOTSUPPORTED; /*XXX*/
		}
		else
		{
				*filesize = fileHandle->fileSize;
		}

		return FAT_ERR_NONE;
}

/* ----------------------------------------------------------------------------------------------------*/

fat_error_t fat_sync (fat_handle_t handle)
{
		fat_file_t * fileHandle;
		fat_error_t result;

		FAT_EMULATE_HOOK_SYNC (handle);

		fileHandle = fat_handle_locate (handle);

		FAT_CHECK_CARD;
		FAT_CHECK_FILESYS;
		FAT_CHECK_HANDLE (fileHandle);

		if ((fileHandle->status & FAT_STATUS_UNSYNC) != FAT_STATUS_NONE)
		{
				if ((result = fat_file_sync (fileHandle)) != FAT_ERR_NONE)
				{
						return result;
				}
		}

		return FAT_ERR_NONE;
}

/* ----------------------------------------------------------------------------------------------------*/

fat_error_t fat_unlink (const char * const filename)
{
		fat_sector_t dirSector = 0;
		fat_offset_t dirOffset = 0;
		fat_cluster_t fileCluster = 0;
		fat_filesize_t fileSize = 0;
		fat_error_t result;

		FAT_EMULATE_HOOK_UNLINK (filename);

		FAT_CHECK_CARD;
		FAT_CHECK_FILESYS;
		FAT_CHECK_FILENAME (filename);

		if ((result = fat_file_locate (filename, &dirSector, &dirOffset, &fileCluster, &fileSize)) == FAT_ERR_NONE)
		{
				if ((result = fat_handle_checkifinuse (fileCluster)) == FAT_ERR_NONE)
				{
						if ((result = fat_file_unlink (dirSector, dirOffset, fileCluster)) == FAT_ERR_NONE)
						{
								result = fat_cluster_sector_sync ();
						}
				}
		}

		return (fat_handle_t) result;
}

/* ----------------------------------------------------------------------------------------------------*/

fat_error_t fat_flush (void)
{
		fat_handle_t handle;
		fat_error_t result;

		FAT_EMULATE_HOOK_FLUSH ();

		FAT_CHECK_CARD;
		FAT_CHECK_FILESYS;

		for (handle = (fat_handle_t) 0; handle < FAT_FILE_SIZE; handle += (fat_handle_t) 1)
		{
				fat_file_t * const fileHandle = fat_handle_locate (handle);

				if (fileHandle != NULL && (fileHandle->status & FAT_STATUS_UNSYNC) != FAT_STATUS_NONE)
				{
						if ((result = fat_file_sync (fileHandle)) != FAT_ERR_NONE)
						{
								return result;
						}
				}
		}

		if ((result = fat_cluster_sector_sync ()) != FAT_ERR_NONE)
		{
				return result;
		}

		return FAT_ERR_NONE;
}

/* ----------------------------------------------------------------------------------------------------*/

#ifdef TEST_ENABLED

static void fat_test_print (void)
{
		fat_handle_t handle;

		DPRINTF (("fatlib:\n"));
				DPRINTF (("\tconfig: %s\n", "none"));

		DPRINTF (("disk-info:\n"));
				DPRINTF (("\tfile-system=%s\n", (fat_disk.fileSys == FAT_FILESYS_FAT16) ? "fat16" : "unknown"));
				DPRINTF (("\tsectors: offset=%lu, reserved=%d, total=%lu\n", fat_disk.sectorsOffset, fat_disk.sectorsReserved, fat_disk.sectorsTotal));
				DPRINTF (("\tfat: sector-offset=%d, sectors-per-fat=%d, number-of-fat=%d, sectors-per-cluster=%d\n",
						fat_disk.sectorsReserved, fat_disk.sectorsPerFAT, fat_disk.numberFAT, fat_disk.sectorsPerCluster));
				DPRINTF (("\tdirectory: sector-offset=%lu, sector-count=%d\n", fat_disk.rootDirectory, fat_disk.rootSectors));
				DPRINTF (("\tdata: sector-offset=%lu\n", fat_disk.dataStarts));
				DPRINTF (("\tcached: fat-sector=%lu, dir-sector=%lu, fat-alloc-offset=%d\n", fat_disk.cachedFATsector, fat_disk.cachedDIRsector,
						fat_disk.cachedFATsector_allocateLastOffset));

		DPRINTF (("file-info:\n"));
				for (handle = (fat_handle_t) 0; handle < FAT_FILE_SIZE; handle += (fat_handle_t) 1)
				{
				const fat_file_t * const fileHandle = fat_handle_locate (handle);
				DPRINTF (("\tfile-handle=%d, active=%s\n", handle, (fileHandle == NULL) ? "no" : "yes"));
				if (fileHandle == NULL)
						continue;
				DPRINTF (("\t\toption: "));
						if ((fileHandle->options & FAT_OPTION_ACTIVE) != FAT_OPTION_NONE) DPRINTF (("active "));
						if ((fileHandle->options & FAT_OPTION_READ) != FAT_OPTION_NONE) DPRINTF (("read "));
						if ((fileHandle->options & FAT_OPTION_WRITE) != FAT_OPTION_NONE) DPRINTF (("write "));
						if ((fileHandle->options & FAT_OPTION_SYNC) != FAT_OPTION_NONE) DPRINTF (("sync "));
				DPRINTF (("\n"));
				DPRINTF (("\t\tdirectory: sector=%lu, offset=%d\n", fileHandle->dirSector, fileHandle->dirOffset));
				DPRINTF (("\t\tfile: cluster=%u, size=%lu\n", fileHandle->fileCluster, fileHandle->fileSize));
				DPRINTF (("\t\tcurrent: cluster=%u, sector=%lu, offset=%d\n", fileHandle->currentCluster, fileHandle->currentSector, fileHandle->currentOffset));
				DPRINTF (("\t\tcached: sector=%lu\n", fileHandle->cachedSector));
				DPRINTF (("\t\tstatus: "));
						if ((fileHandle->status & FAT_STATUS_UNSYNC) != FAT_STATUS_NONE) DPRINTF (("unsync "));
				DPRINTF (("\n"));
				}
}

static const char * fat_test_filename (/*@returned@*/ char * const buffer, const unsigned int offset)
{
		return util_makenumberedfn (buffer, "TEST", "TMP", offset);
}

test_result_t fat_test (void)
{
		fat_handle_t handle, handles [FAT_FILE_SIZE + (fat_handle_t) 1];
		fat_dirent_t dirent;
		char filename [13];
		unsigned int fileindex;
		unsigned int offset;
		unsigned char* buffer = util_buffer_alloc ((unsigned int) 16384);
		unsigned int length = (unsigned int) (16384 >> 1);
		unsigned char* buftmp;
		unsigned int actual;
		unsigned long filesize;

		test_assert (buffer != NULL);

		buftmp = & buffer [length];

		for (offset = 0; offset < length; offset++)
				buffer [offset] = (unsigned char) ((offset + (offset >> 8)) & 0xFF);
		for (fileindex = 0; fileindex < (unsigned int) 990; fileindex++) {
				*filename = '\0';
				handle = fat_open (fat_test_filename (filename, fileindex), FAT_OPT_READ);
				if (!FAT_HANDLE_VALID (handle))
						break;
				test_assert (fat_close (handle) == FAT_ERR_NONE);
		}
		util_rand_seed (offset);

		/* fatfile write */
		/* write open */
		test_assert (!((handle = fat_open (fat_test_filename (filename, fileindex), FAT_OPT_WRITE)) < (fat_handle_t) 0));
		/* write size = SECTOR_SIZE x 3 */
		test_assert (fat_write (handle, &buffer [CARD_SECTOR_SIZE*0], (unsigned int) CARD_SECTOR_SIZE, &actual) == FAT_ERR_NONE && actual == (unsigned int) CARD_SECTOR_SIZE);
		test_assert (fat_write (handle, &buffer [CARD_SECTOR_SIZE*1], (unsigned int) CARD_SECTOR_SIZE, &actual) == FAT_ERR_NONE && actual == (unsigned int) CARD_SECTOR_SIZE);
		test_assert (fat_write (handle, &buffer [CARD_SECTOR_SIZE*2], (unsigned int) CARD_SECTOR_SIZE, &actual) == FAT_ERR_NONE && actual == (unsigned int) CARD_SECTOR_SIZE);
		/* write size = 1 */
		for (offset = (unsigned int) (CARD_SECTOR_SIZE*3); offset < (unsigned int) (CARD_SECTOR_SIZE*3 + 94); offset++)
				test_assert (fat_write (handle, &buffer [offset], (unsigned int) 1, &actual) == FAT_ERR_NONE && actual == (unsigned int) 1);
		/* write size = <rand> */
		for (offset = (unsigned int) (CARD_SECTOR_SIZE*3 + 94); offset < length; ) {
				unsigned int partial = (unsigned int) ((util_rand () % (CARD_SECTOR_SIZE << 1)) + 1);
				if (partial > (length - offset))
						partial = length - offset;
				test_assert (fat_write (handle, &buffer [offset], partial, &actual) == FAT_ERR_NONE && actual == (unsigned int) partial);
				offset += partial;
		}
		/* write invalid */
		test_assert (fat_write (handle, buffer, 0, &actual) != FAT_ERR_NONE);
		/*@-nullpass@*/ test_assert (fat_write (handle, NULL, 0, &actual) != FAT_ERR_NONE); /*@=nullpass@*/
		/*test_assert (fat_write (handle, buffer, CARD_SECTOR_SIZE + 1) < 0);*/
		test_assert (fat_write (handle + (fat_handle_t) 1, buffer, (unsigned int) CARD_SECTOR_SIZE, &actual) != FAT_ERR_NONE);
		/* write close */
		test_assert (fat_close (handle) == FAT_ERR_NONE);
		/*fat_test_print ();*/

		/* fatfile read */
		/* read open */
		test_assert (!((handle = fat_open (fat_test_filename (filename, fileindex), FAT_OPT_READ)) < (fat_handle_t) 0));
		/* read size = SECTOR_SIZE x 3 */
		test_assert (fat_read (handle, buftmp, (unsigned int) CARD_SECTOR_SIZE, &actual) == FAT_ERR_NONE && actual == (unsigned int) CARD_SECTOR_SIZE && util_memcmp (buftmp, &buffer [CARD_SECTOR_SIZE*0], (unsigned int) CARD_SECTOR_SIZE) == 0);
		test_assert (fat_read (handle, buftmp, (unsigned int) CARD_SECTOR_SIZE, &actual) == FAT_ERR_NONE && actual == (unsigned int) CARD_SECTOR_SIZE && util_memcmp (buftmp, &buffer [CARD_SECTOR_SIZE*1], (unsigned int) CARD_SECTOR_SIZE) == 0);
		test_assert (fat_read (handle, buftmp, (unsigned int) CARD_SECTOR_SIZE, &actual) == FAT_ERR_NONE && actual == (unsigned int) CARD_SECTOR_SIZE && util_memcmp (buftmp, &buffer [CARD_SECTOR_SIZE*2], (unsigned int) CARD_SECTOR_SIZE) == 0);
		/* read size = 1 */
		for (offset = (unsigned int) (CARD_SECTOR_SIZE*3); offset < (unsigned int) (CARD_SECTOR_SIZE*4); offset++)
				test_assert (fat_read (handle, buftmp, (unsigned int) 1, &actual) == FAT_ERR_NONE && actual == (unsigned int) 1 && util_memcmp (buftmp, &buffer [offset], (unsigned int) 1) == 0);
		/* read size = <rand> */
		for (offset = (unsigned int) (CARD_SECTOR_SIZE*4); offset < length; ) {
				unsigned int partial = (unsigned int) ((util_rand () % (CARD_SECTOR_SIZE << 1)) + 1);
				if (partial > (length - offset))
						partial = length - offset;
				test_assert (fat_read (handle, buftmp, partial, &actual) == FAT_ERR_NONE && actual == (unsigned int) partial);
				test_assert (util_memcmp (buftmp, &buffer [offset], partial) == 0);
				offset += partial;
		}
		/* read invalid */
		test_assert (fat_read (handle, buftmp, 0, &actual) != FAT_ERR_NONE);
		/*@-nullpass@*/ test_assert (fat_read (handle, NULL, 0, &actual) != FAT_ERR_NONE); /*@=nullpass@*/
		/*test_assert (fat_read (handle, buftmp, CARD_SECTOR_SIZE + 1) < 0);*/
		test_assert (fat_read (handle + (fat_handle_t) 1, buftmp, (unsigned int) CARD_SECTOR_SIZE, &actual) != FAT_ERR_NONE);
		/* read close */
		test_assert (fat_close (handle) == FAT_ERR_NONE);
		/*fat_test_print ();*/

		/* fatfile open/close */
		/* open exclusive */
		test_assert (!((handle = fat_open (fat_test_filename (filename, fileindex), FAT_OPT_READ)) < (fat_handle_t) 0));
		test_assert (fat_open (fat_test_filename (filename, fileindex), FAT_OPT_READ) < (fat_handle_t) 0);
		test_assert (fat_open (fat_test_filename (filename, fileindex), FAT_OPT_WRITE) < (fat_handle_t) 0);

		/* open multiple */
		handles [0] = handle;
		for (handle = (fat_handle_t) 1; handle < (FAT_FILE_SIZE + (fat_handle_t) 1); handle += (fat_handle_t) 1) {
				if (handle < FAT_FILE_SIZE)
						test_assert (!((handles [handle] = fat_open (fat_test_filename (filename, fileindex + (unsigned int) handle), FAT_OPT_WRITE)) < (fat_handle_t) 0));
				else
						test_assert (fat_open (fat_test_filename (filename, fileindex + (unsigned int) handle), FAT_OPT_WRITE) < (fat_handle_t) 0);
		}
		/* close multiple */
		for (handle = (fat_handle_t) 0; handle < FAT_FILE_SIZE; handle += (fat_handle_t) 1)
				test_assert (fat_close (handles [handle]) == FAT_ERR_NONE);
		/*fat_test_print ();*/

		/* unlink */
		for (handle = (fat_handle_t) 0; handle < FAT_FILE_SIZE; handle += (fat_handle_t) 1)
				test_assert (fat_unlink (fat_test_filename (filename, fileindex + (unsigned int) handle)) == FAT_ERR_NONE);

		/* dirent */
		test_assert (!((handle = fat_open ("TEST1.TXT", FAT_OPT_WRITE)) < (fat_handle_t) 0));
		test_assert (fat_write (handle, buffer, (unsigned int) CARD_SECTOR_SIZE, &actual) == FAT_ERR_NONE && actual == (unsigned int) CARD_SECTOR_SIZE);
		test_assert (fat_close (handle) == FAT_ERR_NONE);
		test_assert (!((handle = fat_open ("TESTNAME.TXT", FAT_OPT_WRITE)) < (fat_handle_t) 0));
		test_assert (fat_close (handle) == FAT_ERR_NONE);
		test_assert (!((handle = fat_open ("TESTXX.T", FAT_OPT_WRITE)) < (fat_handle_t) 0));
		test_assert (fat_close (handle) == FAT_ERR_NONE);
		(void) fat_dirent_init (&dirent);
		/*@i@*/ while (fat_dirent_next (&dirent) == FAT_ERR_NONE)
		{
				DPRINTF (("dirent: %s (%lu)\n", dirent.name, dirent.size));
		}
		test_assert (fat_unlink ("TESTXX.T") == FAT_ERR_NONE);
		test_assert (fat_unlink ("TESTNAME.TXT") == FAT_ERR_NONE);
		test_assert (fat_unlink ("TEST1.TXT") == FAT_ERR_NONE);

		/* fatfile sync/flush */
		/* sync updates first */
		test_assert (!((handles [0] = fat_open (fat_test_filename (filename, fileindex + 0), (FAT_OPT_WRITE|FAT_OPT_SYNC))) < (fat_handle_t) 0));
		test_assert (!((handles [1] = fat_open (fat_test_filename (filename, fileindex + 1), (FAT_OPT_WRITE|FAT_OPT_SYNC))) < (fat_handle_t) 0));
		for (offset = 0; offset < (unsigned int) 8; offset++)
				test_assert (fat_write (handles [0], &buffer [(CARD_SECTOR_SIZE / 2) * offset], (unsigned int) (CARD_SECTOR_SIZE / 2), &actual) == FAT_ERR_NONE && actual == (unsigned int) (CARD_SECTOR_SIZE / 2));
		test_assert (fat_close (handles [0]) == FAT_ERR_NONE);
		test_assert (fat_close (handles [1]) == FAT_ERR_NONE);
		/* sync updates second */
		test_assert (!((handles [0] = fat_open (fat_test_filename (filename, fileindex + 0), (FAT_OPT_READ|FAT_OPT_SYNC))) < (fat_handle_t) 0));
		test_assert (fat_read (handles [0], buftmp, (unsigned int) (CARD_SECTOR_SIZE / 2), &actual) == FAT_ERR_NONE && actual == (unsigned int) (CARD_SECTOR_SIZE / 2));
		test_assert (fat_close (handles [0]) == FAT_ERR_NONE);
		test_assert (!((handles [0] = fat_open (fat_test_filename (filename, fileindex + 0), FAT_OPT_READ)) < (fat_handle_t) 0));
		test_assert (fat_read (handles [0], buftmp, (unsigned int) (CARD_SECTOR_SIZE / 2), &actual) == FAT_ERR_NONE && actual == (unsigned int) (CARD_SECTOR_SIZE / 2));
		test_assert (fat_flush () == FAT_ERR_NONE);
		test_assert (fat_close (handles [0]) == FAT_ERR_NONE);
		/*fat_test_print ();*/
		test_assert (fat_unlink (fat_test_filename (filename, fileindex + 0)) == FAT_ERR_NONE);
		test_assert (fat_unlink (fat_test_filename (filename, fileindex + 1)) == FAT_ERR_NONE);

		/* fat sync XXX */

		/* fat_tell */
		test_assert (!((handle = fat_open (fat_test_filename (filename, fileindex), FAT_OPT_WRITE)) < (fat_handle_t) 0));
		test_assert (fat_tell (handle, &filesize) == FAT_ERR_NONE);
		test_assert (filesize == 0);
		test_assert (fat_write (handle, buffer, (unsigned int) (CARD_SECTOR_SIZE / 2), &actual) == FAT_ERR_NONE && actual == (unsigned int) (CARD_SECTOR_SIZE / 2));
		test_assert (fat_tell (handle, &filesize) == FAT_ERR_NONE);
		test_assert (filesize == (unsigned long) (CARD_SECTOR_SIZE / 2));
		test_assert (fat_write (handle, buffer, (unsigned int) (CARD_SECTOR_SIZE + (CARD_SECTOR_SIZE / 2)), &actual) == FAT_ERR_NONE && actual == (unsigned int) (CARD_SECTOR_SIZE + (CARD_SECTOR_SIZE / 2)));
		test_assert (fat_tell (handle, &filesize) == FAT_ERR_NONE);
		test_assert (filesize == (unsigned long) (CARD_SECTOR_SIZE * 2));
		test_assert (fat_close (handle) == FAT_ERR_NONE);
#if 0 /*NOTSUPPORTED*/
		test_assert (!((handle = fat_open (fat_test_filename (filename, fileindex), FAT_OPT_READ)) < (fat_handle_t) 0));
		test_assert (fat_tell (handle, &filesize) == FAT_ERR_NONE);
		test_assert (filesize == 0);
		test_assert (fat_read (handle, buftmp, (unsigned int) (CARD_SECTOR_SIZE / 2), &actual) == FAT_ERR_NONE && actual == (unsigned int) (CARD_SECTOR_SIZE / 2));
		test_assert (fat_read (handle, buftmp, (unsigned int) (CARD_SECTOR_SIZE / 2), &actual) == FAT_ERR_NONE && actual == (unsigned int) (CARD_SECTOR_SIZE / 2));
		test_assert (fat_tell (handle, &filesize) == FAT_ERR_NONE);
		test_assert (filesize == (unsigned long) (CARD_SECTOR_SIZE * 1));
		test_assert (fat_read (handle, buftmp, (unsigned int) (CARD_SECTOR_SIZE / 2), &actual) == FAT_ERR_NONE && actual == (unsigned int) (CARD_SECTOR_SIZE / 2));
		test_assert (fat_read (handle, buftmp, (unsigned int) (CARD_SECTOR_SIZE / 2), &actual) == FAT_ERR_NONE && actual == (unsigned int) (CARD_SECTOR_SIZE / 2));
		test_assert (fat_tell (handle, &filesize) == FAT_ERR_NONE);
		test_assert (filesize == (unsigned long) (CARD_SECTOR_SIZE * 2));
		test_assert (fat_close (handle) == FAT_ERR_NONE);
#endif
		test_assert (fat_unlink (fat_test_filename (filename, fileindex)) == FAT_ERR_NONE);

		/* fat_seek */
		test_assert (!((handle = fat_open (fat_test_filename (filename, fileindex), FAT_OPT_WRITE)) < (fat_handle_t) 0));
		test_assert (fat_write (handle, buffer, (unsigned int) (CARD_SECTOR_SIZE / 2), &actual) == FAT_ERR_NONE && actual == (unsigned int) (CARD_SECTOR_SIZE / 2));
		test_assert (fat_close (handle) == FAT_ERR_NONE);
		test_assert (!((handle = fat_open (fat_test_filename (filename, fileindex), FAT_OPT_WRITE|FAT_OPT_APPEND)) < (fat_handle_t) 0));
		test_assert (fat_write (handle, buffer, (unsigned int) (CARD_SECTOR_SIZE / 2), &actual) == FAT_ERR_NONE && actual == (unsigned int) (CARD_SECTOR_SIZE / 2));
		test_assert (fat_close (handle) == FAT_ERR_NONE);
		test_assert (!((handle = fat_open (fat_test_filename (filename, fileindex), FAT_OPT_READ)) < (fat_handle_t) 0));
		test_assert (fat_read (handle, buftmp, (unsigned int) (CARD_SECTOR_SIZE * 2), &actual) == FAT_ERR_NONE && actual == (unsigned int) (CARD_SECTOR_SIZE * 1));
		test_assert (fat_close (handle) == FAT_ERR_NONE);
		test_assert (!((handle = fat_open (fat_test_filename (filename, fileindex), FAT_OPT_WRITE|FAT_OPT_APPEND)) < (fat_handle_t) 0));
		test_assert (fat_write (handle, buffer, (unsigned int) (CARD_SECTOR_SIZE / 2), &actual) == FAT_ERR_NONE && actual == (unsigned int) (CARD_SECTOR_SIZE / 2));
		test_assert (fat_close (handle) == FAT_ERR_NONE);
		test_assert (!((handle = fat_open (fat_test_filename (filename, fileindex), FAT_OPT_READ)) < (fat_handle_t) 0));
		test_assert (fat_read (handle, buftmp, (unsigned int) (CARD_SECTOR_SIZE * 2), &actual) == FAT_ERR_NONE && actual == (unsigned int) (CARD_SECTOR_SIZE + CARD_SECTOR_SIZE / 2));
		test_assert (fat_close (handle) == FAT_ERR_NONE);
		test_assert (fat_unlink (fat_test_filename (filename, fileindex)) == FAT_ERR_NONE);

		/* suspend & resume, implicit or explicit */

		/* disk error, disk invalid, fat full, dir full	 */

		fat_test_print ();

		util_buffer_reset ();

		return TEST_RESULT_OKAY;
}

#endif

/* ----------------------------------------------------------------------------------------------------*/

