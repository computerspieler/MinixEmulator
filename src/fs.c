#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>

#include "services.h"
#include "macros.h"
#include "fs.h"
#include "fs_param.h"

int fs_interpret_message(Emulator_Env *env, uint32_t dest_src, message *mess, int direction)
{
	int i;
	char c;
	int flags;
	int file_desc;
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

		buf = malloc(sizeof(char) * mess->name1_length);
		if(!buf)
			return -1;

		for(i = 0; i < mess->name1_length; i ++) 
			buf[i] = env->read_byte(env, mess->name1+i);

		if(mess->mode & FS_O_EXCL) flags |= O_EXCL;
		if(mess->mode & FS_O_CREAT) flags |= O_CREAT;
		if(mess->mode & FS_O_TRUNC) flags |= O_TRUNC;
		if(mess->mode & FS_O_NOCTTY) flags |= O_NOCTTY;

		if(mess->mode & FS_O_APPEND) flags |= O_APPEND;
		if(mess->mode & FS_O_NONBLOCK) flags |= O_NONBLOCK;

		if(mess->mode & FS_O_RDONLY) flags |= O_RDONLY;
		if(mess->mode & FS_O_WRONLY) flags |= O_WRONLY;
		if(mess->mode & FS_O_RDWR)   flags |= O_RDWR;

		file_desc = open(buf, flags);
		free(buf);

		return file_desc;

	default:
		FS_ERROR_LOG("Unknown message type: %d\n", mess->m_type);
		return 1;
	}
}
