#include <assert.h>
#include <stdio.h>

#include "services.h"
#include "macros.h"
#include "fs.h"
#include "mm.h"

int interpret_message(Emulator_Env *env, uint32_t dest_src, message *mess, int direction)
{
	assert(env);
	assert(mess);

	DEBUG_LOG("source: %d, dest-src: %d, type: %d, direction: %d\n",
		mess->m_source, dest_src, mess->m_type, direction);

	env->response = *mess;
	switch(dest_src) {
	case MM:
		return mm_interpret_message(env, dest_src, mess, direction);
	case FS:
		return fs_interpret_message(env, dest_src, mess, direction);
	default:
		ERROR_LOG("Unknown destination: %d\n", dest_src);
		return 1;
	}
}
