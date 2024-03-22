#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include "exec_format.h"
#include "macros.h"
#include "i386.h"
#include "array.h"
#include "utils.h"
#include "config.h"

void die()
{
#ifdef DEBUG
    printf("Usage: emulator [LOG FILE] [CHROOT] [EXECUTABLE] [ARGUMENTS]\n");
#else
	printf("Usage: emulator [CHROOT] [EXECUTABLE] [ARGUMENTS]\n");
#endif
	exit(EXIT_FAILURE);
}

int main(int argc, char* argv[])
{
	int ret;
	FILE *fp;
	Emulator_Env env;

#ifdef DEBUG
	if(argc < 4)
#else
	if(argc < 3)
#endif
		die();

#ifdef DEBUG
	if(debug_init(argv[1]))
		return -1;

	argc --;
	argv = &argv[1];
#endif

	if(build_env(&env))
		return -1;

	env.chroot_path = argv[1];
	env.chroot_path_length = strlen(argv[1]);

	fp = fopen(argv[2], "r");
	if(!fp) {
		fprintf(stderr, "Error for path %s: ", argv[2]);
		perror("fopen");
		destroy_env(&env);
		return -1;
	}

	argc -= 2;
	argv = &argv[2];

	if(read_executable(&env, fp)) {
		fclose(fp);
		destroy_env(&env);
		return -1;
	}
	fclose(fp);

	// TODO: Make it configurable
	add_arguments_env(&env, argc, argv, sizeof(envp)/sizeof(char*), envp);

	ret = run_executable(&env);

#ifdef DEBUG
	debug_close();
#endif

	return ret;
}
