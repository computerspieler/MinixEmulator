#ifndef _SERVICES_H_
#define _SERVICES_H_

#include "type.h"

int interpret_message(Emulator_Env *env, uint32_t dest_src, message *mess, int direction);

#endif
