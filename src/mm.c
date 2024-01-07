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
	case GETUID: return getuid();
	case GETPID: return getpid();
	case GETGID:
		env->response.reply_res2 = getegid();
		return getgid();
	case EXIT:
		env->stop = 1;
		env->exit_status = mess->m1_i1;
		return 0;

	case BRK:
		mess->addr += env->data_start;
		MM_DEBUG_LOG("new size: %d\n", mess->addr - env->heap_start);
		if(mess->addr < env->heap_start) {
			MM_DEBUG_LOG("brk refused\n");
			env->response.reply_ptr = 0;
			return 1;
		}
		MM_DEBUG_LOG("brk validated\n");

		array_set_size(&env->heap, mess->addr - env->heap_start);
		env->response.reply_ptr = env->heap_start - env->data_start
            + array_size(&env->heap);
		MM_DEBUG_LOG("New heap configuration: %08x-%08lx\n",
			env->heap_start,
			env->heap_start + array_size(&env->heap)
		);
		return 0;
	
	case SIGACTION:
		env->write_dword(env, mess->sig_osa,
			env->signal_handlers[mess->sig_nr]);
		env->signal_handlers[mess->sig_nr] = mess->sig_nsa;
		return 0;

	default:
		MM_ERROR_LOG("Unknown message type: %d\n", mess->m_type);
		return 1;
	}
}
