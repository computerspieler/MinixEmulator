#ifndef _I386_H_
#define _I386_H_

#include <stdio.h>

#include "exec_format.h"

int run_x86_emulator(int argc, char* argv[], FILE *fp, struct exec header);

#endif
