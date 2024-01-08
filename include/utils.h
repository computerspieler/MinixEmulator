#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>

#include "type.h"

char *get_path(Emulator_Env *env, uint32_t path_ptr, int length);
int read_executable(Emulator_Env *env, FILE *fp);
int build_env(Emulator_Env *env);
void add_arguments_env(Emulator_Env *env, int argc, char* argv[]);
void destroy_env(Emulator_Env *env);
int run_executable(Emulator_Env *env);

#endif /* _UTILS_H_ */
