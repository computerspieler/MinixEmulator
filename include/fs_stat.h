/* The <sys/stat.h> header defines a struct that is used in the stat() and
 * fstat functions.  The information in this struct comes from the i-node of
 * some file.  These calls are the only approved way to inspect i-nodes.
 */

#ifndef _STAT_H
#define _STAT_H

#include "type.h"

struct fs_stat {
	int16_t s_dev;			/* major/minor device number */
	int16_t s_ino;			/* i-node number */
	uint16_t s_mode;		/* file mode, protection bits, etc. */
	int16_t s_nlink;		/* # links; TEMPORARY HACK: should be nlink_t*/
	int16_t s_uid;			/* uid of the file's owner */
	int16_t s_gid;			/* gid; TEMPORARY HACK: should be gid_t */
	int16_t s_rdev;
	uint32_t s_size;		/* file size */
	int32_t s_atime;		/* time of last access */
	int32_t s_mtime;		/* time of last data modification */
	int32_t s_ctime;		/* time of last file status change */
};

/* Traditional mask definitions for s_mode. */
/* The ugly casts on only some of the definitions are to avoid suprising sign
 * extensions such as S_IFREG != (uint16_t) S_IFREG when ints are 32 bits.
 */
#define FS_S_IFMT  ((uint16_t) 0170000)	/* type of file */
#define FS_S_IFREG ((uint16_t) 0100000)	/* regular */
#define FS_S_IFBLK 0060000		/* block special */
#define FS_S_IFDIR 0040000  	/* directory */
#define FS_S_IFCHR 0020000		/* character special */
#define FS_S_IFIFO 0010000		/* this is a FIFO */
#define FS_S_ISUID 0004000		/* set user id on execution */
#define FS_S_ISGID 0002000		/* set group id on execution */
				/* next is reserved for future use */
#define FS_S_ISVTX   01000		/* save swapped text even after use */

/* POSIX masks for s_mode. */
#define FS_S_IRWXU   00700		/* owner:  rwx------ */
#define FS_S_IRUSR   00400		/* owner:  r-------- */
#define FS_S_IWUSR   00200		/* owner:  -w------- */
#define FS_S_IXUSR   00100		/* owner:  --x------ */

#define FS_S_IRWXG   00070		/* group:  ---rwx--- */
#define FS_S_IRGRP   00040		/* group:  ---r----- */
#define FS_S_IWGRP   00020		/* group:  ----w---- */
#define FS_S_IXGRP   00010		/* group:  -----x--- */

#define FS_S_IRWXO   00007		/* others: ------rwx */
#define FS_S_IROTH   00004		/* others: ------r-- */ 
#define FS_S_IWOTH   00002		/* others: -------w- */
#define FS_S_IXOTH   00001		/* others: --------x */

/* The following macros test s_mode (from POSIX Sec. 5.6.1.1). */
#define FS_S_ISREG(m)	(((m) & FS_S_IFMT) == FS_S_IFREG)	/* is a reg file */
#define FS_S_ISDIR(m)	(((m) & FS_S_IFMT) == FS_S_IFDIR)	/* is a directory */
#define FS_S_ISCHR(m)	(((m) & FS_S_IFMT) == FS_S_IFCHR)	/* is a char spec */
#define FS_S_ISBLK(m)	(((m) & FS_S_IFMT) == FS_S_IFBLK)	/* is a block spec */
#define FS_S_ISFIFO(m)	(((m) & FS_S_IFMT) == FS_S_IFIFO)	/* is a pipe/FIFO */

#endif /* _STAT_H */
