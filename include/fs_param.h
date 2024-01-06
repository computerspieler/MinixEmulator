/* The following names are synonyms for the variables in the input message. */
#define acc_time      m2_l1
#define addr	      m1_i3
#define buffer	      m1_p1
#define child	      m1_i2
#define co_mode	      m1_i1
#define eff_grp_id    m1_i3
#define eff_user_id   m1_i3
#define erki          m1_p1
#define fd	      	  m1_i1
#define fd2	      	  m1_i2
#define ioflags       m1_i3
#define group	      m1_i3
#define real_grp_id   m1_i2
#define ls_fd	      m2_i1
#define mk_mode	      m1_i2
#define mode	      m1_i2
#define c_mode        m1_i3
#define c_name        m1_p1
#define name	      m3_p1
#define name1	      m3_p1
#define name2	      m1_p2
#define	name_length   m3_i1
#define name1_length  m1_i1
#define name2_length  m1_i2
#define nbytes        m1_i2
#define offset	      m2_l1
#define owner	      m1_i2
#define parent	      m1_i1
#define pathname      m3_ca1
#define pid			  m1_i3
#define pro			  m1_i1
#define rd_only	      m1_i3
#define real_user_id  m1_i2
#define request       m1_i2
#define sig	          m1_i2
#define slot1	      m1_i1
#define tp	          m2_l1
#define utime_actime  m2_l1
#define utime_modtime m2_l2
#define utime_file    m2_p1
#define utime_length  m2_i1
#define whence	      m2_i2
#define svrctl_req    m2_i1
#define svrctl_argp   m2_p1

/* The following names are synonyms for the variables in the output message. */
#define reply_type    m_type
#define reply_l1      m2_l1
#define reply_i1      m1_i1
#define reply_i2      m1_i2
#define reply_t1      m4_l1
#define reply_t2      m4_l2
#define reply_t3      m4_l3
#define reply_t4      m4_l4
#define reply_t5      m4_l5

/* These values are used for cmd in fcntl().  POSIX Table 6-1.  */
#define FS_F_DUPFD            0	/* duplicate file descriptor */
#define FS_F_GETFD	          1	/* get file descriptor flags */
#define FS_F_SETFD            2	/* set file descriptor flags */
#define FS_F_GETFL            3	/* get file status flags */
#define FS_F_SETFL            4	/* set file status flags */
#define FS_F_GETLK            5	/* get record locking information */
#define FS_F_SETLK            6	/* set record locking information */
#define FS_F_SETLKW           7	/* set record locking info; wait if blocked */

/* File descriptor flags used for fcntl().  POSIX Table 6-2. */
#define FS_FD_CLOEXEC         1	/* close on exec flag for third arg of fcntl */

/* L_type values for record locking with fcntl().  POSIX Table 6-3. */
#define FS_F_RDLCK            1	/* shared or read lock */
#define FS_F_WRLCK            2	/* exclusive or write lock */
#define FS_F_UNLCK            3	/* unlock */

/* Oflag values for open().  POSIX Table 6-4. */
#define FS_O_CREAT        00100	/* creat file if it doesn't exist */
#define FS_O_EXCL         00200	/* exclusive use flag */
#define FS_O_NOCTTY       00400	/* do not assign a controlling terminal */
#define FS_O_TRUNC        01000	/* truncate flag */

/* File status flags for open() and fcntl().  POSIX Table 6-5. */
#define FS_O_APPEND       02000	/* set append mode */
#define FS_O_NONBLOCK     04000	/* no delay */

/* File access modes for open() and fcntl().  POSIX Table 6-6. */
#define FS_O_RDONLY           0	/* open(name, O_RDONLY) opens read only */
#define FS_O_WRONLY           1	/* open(name, O_WRONLY) opens write only */
#define FS_O_RDWR             2	/* open(name, O_RDWR) opens read/write */

/* Mask for use with file access modes.  POSIX Table 6-7. */
#define FS_O_ACCMODE         03	/* mask for file access modes */
