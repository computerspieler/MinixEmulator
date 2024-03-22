/* The <errno.h> header defines the numbers of the various errors that can
 * occur during program execution.  They are visible to user programs and 
 * should be small positive integers.  However, they are also used within 
 * MINIX, where they must be negative.  For example, the READ system call is 
 * executed internally by calling do_read().  This function returns either a 
 * (negative) error number or a (positive) number of bytes actually read.
 *
 * To solve the problem of having the error numbers be negative inside the
 * the system and positive outside, the following mechanism is used.  All the
 * definitions are are the form:
 *
 *	#define EPERM		(_SIGN 1)
 *
 * If the macro _SYSTEM is defined, then  _SIGN is set to "-", otherwise it is
 * set to "".  Thus when compiling the operating system, the  macro _SYSTEM
 * will be defined, setting EPERM to (- 1), whereas when when this
 * file is included in an ordinary user program, EPERM has the value ( 1).
 */

#ifndef _MINIX_ERRNO_H		/* check if <errno.h> is already included */
#define _MINIX_ERRNO_H		/* it is not included; note that fact */

#include <errno.h>

/* Here are the numerical values of the error numbers. */
#define _NERROR               70  /* number of errors */  

#define MINIX_EOK           (0)
#define MINIX_EGENERIC      (99)  /* generic error */
#define MINIX_EPERM         (1)   /* operation not permitted */
#define MINIX_ENOENT        (2)   /* no such file or directory */
#define MINIX_ESRCH         (3)   /* no such process */
#define MINIX_EINTR         (4)   /* interrupted function call */
#define MINIX_EIO           (5)   /* input/output error */
#define MINIX_ENXIO         (6)   /* no such device or address */
#define MINIX_E2BIG         (7)   /* arg list too long */
#define MINIX_ENOEXEC       (8)   /* exec format error */
#define MINIX_EBADF         (9)   /* bad file descriptor */
#define MINIX_ECHILD        (10)  /* no child process */
#define MINIX_EAGAIN        (11)  /* resource temporarily unavailable */
#define MINIX_ENOMEM        (12)  /* not enough space */
#define MINIX_EACCES        (13)  /* permission denied */
#define MINIX_EFAULT        (14)  /* bad address */
#define MINIX_ENOTBLK       (15)  /* Extension: not a block special file */
#define MINIX_EBUSY         (16)  /* resource busy */
#define MINIX_EEXIST        (17)  /* file exists */
#define MINIX_EXDEV         (18)  /* improper link */
#define MINIX_ENODEV        (19)  /* no such device */
#define MINIX_ENOTDIR       (20)  /* not a directory */
#define MINIX_EISDIR        (21)  /* is a directory */
#define MINIX_EINVAL        (22)  /* invalid argument */
#define MINIX_ENFILE        (23)  /* too many open files in system */
#define MINIX_EMFILE        (24)  /* too many open files */
#define MINIX_ENOTTY        (25)  /* inappropriate I/O control operation */
#define MINIX_ETXTBSY       (26)  /* no longer used */
#define MINIX_EFBIG         (27)  /* file too large */
#define MINIX_ENOSPC        (28)  /* no space left on device */
#define MINIX_ESPIPE        (29)  /* invalid seek */
#define MINIX_EROFS         (30)  /* read-only file system */
#define MINIX_EMLINK        (31)  /* too many links */
#define MINIX_EPIPE         (32)  /* broken pipe */
#define MINIX_EDOM          (33)  /* domain error    	(from ANSI C std) */
#define MINIX_ERANGE        (34)  /* result too large	(from ANSI C std) */
#define MINIX_EDEADLK       (35)  /* resource deadlock avoided */
#define MINIX_ENAMETOOLONG  (36)  /* file name too long */
#define MINIX_ENOLCK        (37)  /* no locks available */
#define MINIX_ENOSYS        (38)  /* function not implemented */
#define MINIX_ENOTEMPTY     (39)  /* directory not empty */

/* The following errors relate to networking. */
#define MINIX_EPACKSIZE     (50)  /* invalid packet size for some protocol */
#define MINIX_EOUTOFBUFS    (51)  /* not enough buffers left */
#define MINIX_EBADIOCTL     (52)  /* illegal ioctl for device */
#define MINIX_EBADMODE      (53)  /* badmode in ioctl */
#define MINIX_EWOULDBLOCK   (54)
#define MINIX_EBADDEST      (55)  /* not a valid destination address */
#define MINIX_EDSTNOTRCH    (56)  /* destination not reachable */
#define MINIX_EISCONN	    (57)  /* all ready connected */
#define MINIX_EADDRINUSE    (58)  /* address in use */
#define MINIX_ECONNREFUSED  (59)  /* connection refused */
#define MINIX_ECONNRESET    (60)  /* connection reset */
#define MINIX_ETIMEDOUT     (61)  /* connection timed out */
#define MINIX_EURG	      	(62)  /* urgent data present */
#define MINIX_ENOURG	    (63)  /* no urgent data present */
#define MINIX_ENOTCONN      (64)  /* no connection (yet or anymore) */
#define MINIX_ESHUTDOWN     (65)  /* a write call to a shutdown connection */
#define MINIX_ENOCONN       (66)  /* no such connection */

/* The following are not POSIX errors, but they can still happen. */
#define MINIX_ELOCKED      (101)  /* can't send message */
#define MINIX_EBADCALL     (102)  /* error on send/receive */

#endif /* _ERRNO_H */
