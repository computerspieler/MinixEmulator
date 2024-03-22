#include <assert.h>
#include <stdio.h>
#include <errno.h>

#include "services.h"
#include "macros.h"
#include "fs.h"
#include "mm.h"
#include "minix_errno.h"

int interpret_message(Emulator_Env *env, uint32_t dest_src, message *mess, int direction)
{
	assert(env);
	assert(mess);

	//DEBUG_LOG("source: %d, dest-src: %d, type: %d, direction: %d\n",
	//	mess->m_source, dest_src, mess->m_type, direction);

	env->response = *mess;
	env->error_no = MINIX_EOK;
	switch(dest_src) {
	case MM:
		env->response.m_type = mm_interpret_message(env, dest_src, mess, direction);
		break;
	case FS:
		env->response.m_type = fs_interpret_message(env, dest_src, mess, direction);
		break;
	default:
		ERROR_LOG("Unknown destination: %d\n", dest_src);
		return 1;
	}
	
	if(env->error_no != MINIX_EOK)
		env->response.m_type = -env->error_no;
	
	return 0;
}
