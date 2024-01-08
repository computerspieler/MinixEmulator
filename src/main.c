#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include "exec_format.h"
#include "macros.h"
#include "i386.h"
#include "array.h"
#include "utils.h"

void die()
{
    DEBUG_LOG("Usage: emulator [CHROOT] [EXECUTABLE] [ARGUMENTS]\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char* argv[])
{
	FILE *fp;
	Emulator_Env env;

	if(argc < 3)
		die();

	if(build_env(&env))
		return -1;

	env.chroot_path = argv[1];
	env.chroot_path_length = strlen(argv[1]);

	fp = fopen(argv[2], "r");
	if(!fp) {
		perror("fopen");
		destroy_env(&env);
		return -1;
	}

	if(read_executable(&env, fp)) {
		fclose(fp);
		destroy_env(&env);
		return -1;
	}
	fclose(fp);

	add_arguments_env(&env, argc-2, &argv[2]);
	return run_executable(&env);
}
