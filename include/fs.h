#ifndef _FS_H_
#define _FS_H_

#include "type.h"

int fs_interpret_message(Emulator_Env *env, uint32_t dest_src, message *mess, int direction);

#endif
