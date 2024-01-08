#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#include "services.h"
#include "macros.h"
#include "fs.h"
#include "fs_stat.h"
#include "fs_param.h"
#include "utils.h"

#define WRITE_IF_POSSIBLE(base_address, index, max_index, data)	\
	if((index) < (max_index)) {	\
		env->write_byte(env, (base_address) + (index), (data));	\
	}

int fs_interpret_message(Emulator_Env *env, uint32_t dest_src, message *mess, int direction)
{
	int i, j;
	char c;
	int flags, cmd;
	int ret;
	char *buf;
	struct fs_stat fs_statbuf;
	struct stat statbuf;

	DIR* directory;
	struct dirent *dir_ent;
	int dir_ent_name_length;

	FS_DEBUG_LOG("Message type: %d\n", mess->m_type);
	switch(mess->m_type) {
	case IOCTL:
		FS_DEBUG_LOG("Line %x;\n", mess->TTY_LINE);
		array_set(&env->file_handlers, 1, stdout);
		
		env->response.m_type   = DEV_IOCTL;
		env->response.DEVICE   = (mess->TTY_LINE >> MINOR) & 0xFF;
		env->response.PROC_NR  = mess->m_source;
		env->response.ADDRESS  = mess->ADDRESS;
		env->response.POSITION = 0;
		env->response.COUNT    = mess->TTY_REQUEST;
		return 0;
	
	case WRITE:
		mess->nbytes &= 0xFFFF;
		FS_DEBUG_LOG("fd %x; Buffer: %x; Size: %x\n",
			mess->fd, mess->buffer,
			mess->nbytes);
		array_set(&env->file_handlers, 1, stdout);
		
		// Send everything to stderr, it's easier to debug
		if(mess->fd == 1)
			mess->fd = 2;

		for(i = 0; i < mess->nbytes; i ++) {
			c = env->read_byte(env, mess->buffer+i);
			if(!write(mess->fd, &c, 1))
				break;
		}
		
		env->response.m_type = 0;
		env->response.PROC_NR = mess->m_source;
		return i;
	
	case READ:
		mess->nbytes &= 0xFFFF;
		FS_DEBUG_LOG("fd %x; Buffer: %x; Size: %x\n",
			mess->fd, mess->buffer,
			mess->nbytes);
		array_set(&env->file_handlers, 1, stdout);
		
		if(fstat(mess->fd, &statbuf) < 0)
			return 0;

		if(S_ISDIR(statbuf.st_mode)) {
			directory = fdopendir(mess->fd);

			i = 0;

			//FIXME: There might be some issues if the buffer's length
			// isn't a multiple of 16
			while((dir_ent = readdir(directory)) != NULL) {
				/*
					This is the structure used for a
					directory's entry :
					struct _v7_direct {		
						unsigned short	d_ino;
						char			d_name[14];
					};
				*/
				WRITE_IF_POSSIBLE(mess->buffer, i, mess->nbytes,
					dir_ent->d_ino & 0xFF);
				i ++;
				WRITE_IF_POSSIBLE(mess->buffer, i, mess->nbytes,
					(dir_ent->d_ino >> 8) & 0xFF);
				i ++;

				dir_ent_name_length = strlen(dir_ent->d_name);
				for(j = 0; j < MIN(dir_ent_name_length, 14); j ++, i ++) {
					WRITE_IF_POSSIBLE(mess->buffer, i, mess->nbytes,
						dir_ent->d_name[j]);
				}
				for(; j < 14; j ++, i ++) {
					WRITE_IF_POSSIBLE(mess->buffer, i, mess->nbytes, 0);
				}
				
				if(i >= mess->nbytes)
					break;
			}
		} else {
			for(i = 0; i < mess->nbytes; i ++) {
				if(!read(mess->fd, &c, 1))
					break;
				env->write_byte(env, mess->buffer+i, c);
			}

			env->response.m_type = 0;
			env->response.PROC_NR = mess->m_source;
		}

		return i;

	case TIME:
		env->response.reply_l1 = (cpu_ptr) time(NULL);
		return 0;

	case CLOSE:
		return close(mess->fd);

	case OPEN:
		flags = 0;

		if(mess->mode & FS_O_CREAT)
			buf = get_path(env, mess->buffer, mess->name1_length);
		else
			buf = get_path(env, mess->name, mess->name_length);

		if(!buf)
			return -1;

		if(mess->mode & FS_O_EXCL) flags |= O_EXCL;
		if(mess->mode & FS_O_CREAT) flags |= O_CREAT;
		if(mess->mode & FS_O_TRUNC) flags |= O_TRUNC;
		if(mess->mode & FS_O_NOCTTY) flags |= O_NOCTTY;

		if(mess->mode & FS_O_APPEND) flags |= O_APPEND;
		if(mess->mode & FS_O_NONBLOCK) flags |= O_NONBLOCK;

		if(mess->mode & FS_O_RDONLY) flags |= O_RDONLY;
		if(mess->mode & FS_O_WRONLY) flags |= O_WRONLY;
		if(mess->mode & FS_O_RDWR)   flags |= O_RDWR;

		ret = open(buf, flags);

		free(buf);

		return ret;

	case FCNTL:
		switch(mess->request) {
		case FS_F_DUPFD:  cmd = F_DUPFD;  break;
		case FS_F_GETFD:  cmd = F_GETFD;  break;
		case FS_F_SETFD:  cmd = F_SETFD;  break;
		case FS_F_GETFL:  cmd = F_GETFL;  break;
		case FS_F_SETFL:  cmd = F_SETFL;  break;
		case FS_F_GETLK:  cmd = F_GETLK;  break;
		case FS_F_SETLK:  cmd = F_SETLK;  break;
		case FS_F_SETLKW: cmd = F_SETLKW; break;
		}

		return fcntl(mess->fd, cmd);

	case MKDIR:
		buf = get_path(env, mess->buffer, mess->name1_length);
		if(!buf)
			return -1;

		ret = mkdir(buf, mess->mode);
		free(buf);

		return ret;

	case RMDIR:
		buf = get_path(env, mess->name, mess->name_length);
		if(!buf)
			return -1;

		ret = rmdir(buf);
		free(buf);

		return ret;

	case UMASK:
		return umask(mess->co_mode);

	case UNLINK:
		buf = get_path(env, mess->name, mess->name_length);
		if(!buf)
			return -1;

		ret = unlink(buf);
		free(buf);

		return ret;
	
	case LSEEK:
		return lseek(mess->ls_fd, mess->offset, mess->whence);

	case ACCESS:
		buf = get_path(env, mess->name, mess->name_length);
		if(!buf)
			return -1;
		ret = access(buf, mess->m3_i2);
		free(buf);

		return ret;

	case STAT:
		buf = get_path(env, mess->buffer, mess->name1_length);
		if(!buf)
			return -1;

		ret = stat(buf, &statbuf);
		free(buf);

		fs_statbuf.s_dev   = statbuf.st_dev;
		fs_statbuf.s_ino   = statbuf.st_ino;
		fs_statbuf.s_mode  = statbuf.st_mode;
		fs_statbuf.s_nlink = statbuf.st_nlink;
		fs_statbuf.s_uid   = statbuf.st_uid;
		fs_statbuf.s_gid   = statbuf.st_gid;
		fs_statbuf.s_rdev  = statbuf.st_rdev;
		fs_statbuf.s_size  = statbuf.st_size;
		fs_statbuf.s_atime = statbuf.st_atim.tv_sec;
		fs_statbuf.s_mtime = statbuf.st_mtim.tv_sec;
		fs_statbuf.s_ctime = statbuf.st_ctim.tv_sec;

		for(i = 0; i < (int) sizeof(struct fs_stat); i ++)
			env->write_byte(env, mess->m1_p2+i, ((uint8_t*)&fs_statbuf)[i]);

		return 0;

	case FSTAT:
		ret = fstat(mess->fd, &statbuf);

		fs_statbuf.s_dev = statbuf.st_dev;
		fs_statbuf.s_ino = statbuf.st_ino;
		fs_statbuf.s_mode = statbuf.st_mode;
		fs_statbuf.s_nlink = statbuf.st_nlink;
		fs_statbuf.s_uid = statbuf.st_uid;
		fs_statbuf.s_gid = statbuf.st_gid;
		fs_statbuf.s_rdev = statbuf.st_rdev;
		fs_statbuf.s_size = statbuf.st_size;
		fs_statbuf.s_atime = statbuf.st_atim.tv_sec;
		fs_statbuf.s_mtime = statbuf.st_mtim.tv_sec;
		fs_statbuf.s_ctime = statbuf.st_ctim.tv_sec;

		for(i = 0; i < (int) sizeof(struct fs_stat); i ++)
			env->write_byte(env, mess->m1_p1+i, ((uint8_t*)&fs_statbuf)[i]);

		return ret;

	default:
		FS_ERROR_LOG("Unknown message type: %d\n", mess->m_type);
		return 1;
	}
}
