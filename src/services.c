#include <assert.h>
#include <stdio.h>

#include "services.h"
#include "com.h"
#include "callnr.h"

int fs_interpret_message(emulator_env *env, uint32_t dest_src, message *mess)
{
	fprintf(stderr, "[FS] Message type: %d\n", mess->m_type);
	switch(mess->m_type) {
	default:
		fprintf(stderr, "[FS] Unknown message type: %d\n", mess->m_type);
		return 1;
	}
}

int mm_interpret_message(emulator_env *env, uint32_t dest_src, message *mess)
{
	fprintf(stderr, "[MM] Message type: %d\n", mess->m_type);
	switch(mess->m_type) {
	case EXIT:
		env->stop = 1;
		env->exit_status = mess->m1_i1;
		return 0;
	default:
		fprintf(stderr, "[MM] Unknown message type: %d\n", mess->m_type);
		return 1;
	}
}

int interpret_message(emulator_env *env, uint32_t dest_src, message *mess)
{
	assert(env);
	assert(mess);

	printf("source: %8x, type: %d\n", mess->m_source, mess->m_type);

	switch(dest_src) {
	case MM:
		return mm_interpret_message(env, dest_src, mess);
	case FS:
		return fs_interpret_message(env, dest_src, mess);
	default:
		fprintf(stderr, "Unknown destination: %d\n", dest_src);
		return 1;
	}
}
