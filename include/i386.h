#ifndef _I386_H_
#define _I386_H_

#include <stdio.h>

#include "exec_format.h"
#include "type.h"

int run_x86_emulator(emulator_env program_env, FILE *fp, struct exec header);

#endif
