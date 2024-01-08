#ifndef _I386_H_
#define _I386_H_

#include <stdio.h>

#include "exec_format.h"
#include "type.h"

void x86_emulator_init(Emulator_Env *env, int argc, char* argv[]);
int run_x86_emulator(Emulator_Env *env);

#endif
