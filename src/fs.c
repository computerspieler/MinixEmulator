#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#include "services.h"
#include "macros.h"
#include "fs.h"
#include "fs_param.h"

int fs_interpret_message(Emulator_Env *env, uint32_t dest_src, message *mess, int direction)
{
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
		FS_DEBUG_LOG("Line %x; Buffer: %x; Size: %x\n",
			mess->TTY_LINE, mess->buffer,
			mess->nbytes);
		array_set(&env->file_handlers, 1, stdout);
		
		env->response.DEVICE = (mess->TTY_LINE >> MINOR) & 0xFF;
		assert(env->response.DEVICE == 1);
		for(int i = 0; i < mess->nbytes; i ++)
			fputc(env->read_byte(env, mess->buffer), stdout);

		env->response.m_type = 0;
		env->response.PROC_NR  = mess->m_source;
		return 0;

	default:
		FS_ERROR_LOG("Unknown message type: %d\n", mess->m_type);
		return 1;
	}
}
