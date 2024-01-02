#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#include "services.h"
#include "macros.h"
#include "mm.h"
#include "mm_param.h"

int mm_interpret_message(Emulator_Env *env, uint32_t dest_src, message *mess, int direction)
{
	MM_DEBUG_LOG("Message type: %d\n", mess->m_type);
	switch(mess->m_type) {
	case GETUID:
		return getuid();
	case EXIT:
		env->stop = 1;
		env->exit_status = mess->m1_i1;
		return 0;
	case BRK:
		MM_DEBUG_LOG("new_addr: %x\n", (uint32_t) mess->addr);
		if(mess->addr < env->heap_start || mess->addr > 0x80000000)
			return 1;

		env->response.reply_ptr = env->heap_start + array_size(&env->heap);
		array_set_size(&env->heap, mess->addr);
		return 0;
	default:
		MM_ERROR_LOG("Unknown message type: %d\n", mess->m_type);
		return 1;
	}
}
