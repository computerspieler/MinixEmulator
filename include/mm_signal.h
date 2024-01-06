/* The <signal.h> header defines all the ANSI and POSIX signals.
 * MINIX supports all the signals required by POSIX. They are defined below.
 * Some additional signals are also supported.
 */

#ifndef _SIGNAL_H
#define _SIGNAL_H

#include "type.h"

/* Here are types that are closely associated with signal handling. */
typedef cpu_int sig_atomic_t;
typedef uint32_t sigset_t;

#define MM_SIGHUP             1	/* hangup */
#define MM_SIGINT             2	/* interrupt (DEL) */
#define MM_SIGQUIT            3	/* quit (ASCII FS) */
#define MM_SIGILL             4	/* illegal instruction */
#define MM_SIGTRAP            5	/* trace trap (not reset when caught) */
#define MM_SIGABRT            6	/* IOT instruction */
#define MM_SIGIOT             6	/* SIGABRT for people who speak PDP-11 */
#define MM_SIGUNUSED          7	/* spare code */
#define MM_SIGFPE             8	/* floating point exception */
#define MM_SIGKILL            9	/* kill (cannot be caught or ignored) */
#define MM_SIGUSR1           10	/* user defined signal # 1 */
#define MM_SIGSEGV           11	/* segmentation violation */
#define MM_SIGUSR2           12	/* user defined signal # 2 */
#define MM_SIGPIPE           13	/* write on a pipe with no one to read it */
#define MM_SIGALRM           14	/* alarm clock */
#define MM_SIGTERM           15	/* software termination signal from kill */
#define MM_SIGCHLD           17	/* child process terminated or stopped */

#define MM_SIGEMT             7	/* obsolete */
#define MM_SIGBUS            10	/* obsolete */

/* POSIX requires the following signals to be defined, even if they are
 * not supported.  Here are the definitions, but they are not supported.
 */
#define MM_SIGCONT           18	/* continue if stopped */
#define MM_SIGSTOP           19	/* stop signal */
#define MM_SIGTSTP           20	/* interactive stop signal */
#define MM_SIGTTIN           21	/* background process wants to read */
#define MM_SIGTTOU           22	/* background process wants to write */

/* Macros used as function pointers. */
#define MM_SIG_ERR    (-1)	/* error return */
#define MM_SIG_DFL	  ( 0)	/* default signal handling */
#define MM_SIG_IGN	  ( 1)	/* ignore signal */
#define MM_SIG_HOLD   ( 2)	/* block signal */
#define MM_SIG_CATCH  ( 3)	/* catch signal */

/* POSIX requires these values for use with sigprocmask(2). */
#define MM_SIG_BLOCK          0	/* for blocking signals */
#define MM_SIG_UNBLOCK        1	/* for unblocking signals */
#define MM_SIG_SETMASK        2	/* for setting the signal mask */
#define MM_SIG_INQUIRE        4	/* for internal use only */

#endif /* _SIGNAL_H */
