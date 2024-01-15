#include <assert.h>
#include <stdio.h>
#include <errno.h>

#include "services.h"
#include "macros.h"
#include "fs.h"
#include "mm.h"

int interpret_message(Emulator_Env *env, uint32_t dest_src, message *mess, int direction)
{
	int ret;

	assert(env);
	assert(mess);

	//DEBUG_LOG("source: %d, dest-src: %d, type: %d, direction: %d\n",
	//	mess->m_source, dest_src, mess->m_type, direction);

	env->response = *mess;
	switch(dest_src) {
	case MM:
		ret = mm_interpret_message(env, dest_src, mess, direction);
		break;
	case FS:
		ret = fs_interpret_message(env, dest_src, mess, direction);
		break;
	default:
		ERROR_LOG("Unknown destination: %d\n", dest_src);
		return -1;
	}
	// Not sure if that's correct
	// Maybe the value between the host's errno
	// and Minix's errno could be different
	env->response.m_type = errno;
	return ret;
}
