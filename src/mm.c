#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "services.h"
#include "macros.h"
#include "mm.h"
#include "mm_param.h"
#include "utils.h"

int mm_interpret_message(Emulator_Env *env, uint32_t dest_src, message *mess, int direction)
{
	int i, c;
	FILE *f;
	char *buf;
	pid_t ret_pid;
	int ret_status;
	Emulator_Env new_env;

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
	
	case FORK:
		return fork();
	
	case WAIT:
		ret_pid = wait(&ret_status);
		env->response.m2_i1 = ret_status;
		return ret_pid;

	case EXEC:
		buf = get_path(env, mess->exec_name, mess->exec_len);
		if(!buf)
			return -1;

		f = fopen(buf, "r");
		free(buf);

		if(!f) {
			perror("fopen");
			return -1;
		}

		build_env(&new_env);
		if(read_executable(&new_env, f)) {
			fclose(f);
			destroy_env(&new_env);
			return -1;
		}

		fclose(f);

		array_clear(&new_env.stack);
		array_clear(&new_env.heap);

		new_env.chroot_path = env->chroot_path;
		new_env.chroot_path_length = env->chroot_path_length;
		
		for(i = mess->stack_bytes-1; i >= 0; i --) {
			c = env->read_byte(env, mess->stack_ptr + i);
			array_push(&new_env.stack, &c);
		}

		destroy_env(env);

		exit(run_executable(&new_env));

	default:
		MM_ERROR_LOG("Unknown message type: %d\n", mess->m_type);
		return 1;
	}
}
