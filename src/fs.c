#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "services.h"
#include "macros.h"
#include "fs.h"
#include "fs_param.h"

int fs_interpret_message(Emulator_Env *env, uint32_t dest_src, message *mess, int direction)
{
	int i;
	char c;

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

	default:
		FS_ERROR_LOG("Unknown message type: %d\n", mess->m_type);
		return 1;
	}
}
