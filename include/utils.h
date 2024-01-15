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
int get_stat_from_path(char *path, struct fs_stat *out_statbuf);
int get_stat(int fd, struct fs_stat *ret);

#endif /* _UTILS_H_ */
