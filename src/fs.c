#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "services.h"
#include "macros.h"
#include "fs.h"
#include "fs_param.h"

char *get_path(Emulator_Env *env, uint32_t path_ptr, int length)
{
	int i;
	int off;
	int absolute;
	char *buf;

	if(env->read_byte(env, path_ptr) == '/') {
		absolute = 1;
		off = env->chroot_path_length;
	} else {
		absolute = 0;
		off = 0;
	}

	buf = malloc(sizeof(char) * (length + off));
	if(!buf)
		return NULL;

	if(absolute)
		memcpy(buf, env->chroot_path, env->chroot_path_length);

	for(i = 0; i < length; i ++) 
		buf[i+off] = env->read_byte(env, path_ptr+i);


	return buf;
}

int fs_interpret_message(Emulator_Env *env, uint32_t dest_src, message *mess, int direction)
{
	int i;
	char c;
	int flags, cmd;
	int ret;
	char *buf;

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
		
		for(i = 0; i < mess->nbytes; i ++) {
			if(!read(mess->fd, &c, 1))
				break;
			env->write_byte(env, mess->buffer+i, c);
		}

		env->response.m_type = 0;
		env->response.PROC_NR = mess->m_source;
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

	default:
		FS_ERROR_LOG("Unknown message type: %d\n", mess->m_type);
		return 1;
	}
}
