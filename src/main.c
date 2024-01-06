#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include "exec_format.h"
#include "macros.h"
#include "i386.h"
#include "array.h"

void die()
{
    DEBUG_LOG("Usage: emulator [EXECUTABLE] [ARGUMENTS]\n");
	exit(EXIT_FAILURE);
}

int read_executable(Emulator_Env *env, FILE *fp)
{
	unsigned int i;
	struct nlist symbol;

	if(!fread(&env->hdr, A_MINHDR, 1, fp)) {
		perror("fread");
		return -1;
	}

	if(env->hdr.a_hdrlen - A_MINHDR > 0) {
		fseek(fp, 0, SEEK_SET);
		if(!fread(&env->hdr, env->hdr.a_hdrlen, 1, fp)) {
			perror("fread");
			return -1;
		}
	}

	if(BADMAG(env->hdr)) {
		ERROR_LOG("Invalid header\n");
		return -1;
	}
	if(env->hdr.a_flags & ~(A_NSYM | A_EXEC | A_SEP)) {
		ERROR_LOG("Unsupported file\n");
		return -1;
	}

	env->text = malloc(env->hdr.a_text);
	if(!env->text) {
		perror("malloc");
		return -1;
	}

	if(!fread(env->text, env->hdr.a_text, 1, fp)) {
		perror("fread");
		return -1;
	}

	if(!(env->hdr.a_flags & A_SEP))
		env->data = env->text;
	else {
		env->data = malloc(env->hdr.a_data);
		if(!env->data) {
			perror("malloc");
			return -1;
		}

		if(!fread(env->data, env->hdr.a_data, 1, fp)) {
			perror("fread");
			return -1;
		}
	}

	env->bss = malloc(env->hdr.a_bss);
	bzero(env->bss, env->hdr.a_bss);

	if(A_SYMPOS(env->hdr)) {
		fseek(fp, A_SYMPOS(env->hdr), SEEK_SET);

		DEBUG_LOG("=== Symbols ===\n");
		for(i = 0; i < env->hdr.a_syms / sizeof(struct nlist); i ++) {
			if(!fread(&symbol, sizeof(struct nlist), 1, fp)) {
				perror("fread");
				return -1;
			}

			array_push(&env->symbols, &symbol);
			DEBUG_LOG("[%s, %s] \"%.*s\" : %08x\n",
				(
					((symbol.n_sclass & N_SECT) == N_UNDF) ? "Undf" :
					((symbol.n_sclass & N_SECT) == N_ABS ) ? "Abs "  :
					((symbol.n_sclass & N_SECT) == N_TEXT) ? "Code" :
					((symbol.n_sclass & N_SECT) == N_DATA) ? "Data" :
					((symbol.n_sclass & N_SECT) == N_BSS ) ? "BSS "  :
					((symbol.n_sclass & N_SECT) == N_COMM) ? "Comm" :
					"Unknown"
				),
				((symbol.n_sclass & N_CLASS) == C_STAT) ? "static" : "extern",
				8, symbol.n_name,
				symbol.n_value
			);
		}
	}

	return 0;
}

int build_env(Emulator_Env *env, int argc, char* argv[])
{
	bzero(env, sizeof(Emulator_Env));
	
	env->argc = argc-1;	
	env->argv = &argv[1];
	env->file_handlers = array_create(sizeof(FILE*));	
	env->stack = array_create(sizeof(char));
	env->heap = array_create(sizeof(char));
	env->symbols = array_create(sizeof(struct nlist));
	
	return 0;
}

void destroy_env(Emulator_Env *env)
{
	array_free(&env->file_handlers);
	array_free(&env->stack);
	array_free(&env->heap);
	array_free(&env->symbols);
	if(env->text)
		free(env->text);
	if(env->data)
		free(env->data);
	if(env->bss)
		free(env->bss);
}

int main(int argc, char* argv[])
{
	int return_code;
	FILE *fp;
	Emulator_Env env;

	if(argc < 2)
		die();

	if(build_env(&env, argc, argv))
		return -1;

	fp = fopen(argv[1], "r");
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

	switch(env.hdr.a_cpu) {
	case A_I80386:
		DEBUG_LOG("CPU ID: I386\n");
		return_code = run_x86_emulator(&env);
		destroy_env(&env);
		return return_code;
	case A_I8086:
		DEBUG_LOG("Targeted CPU: I86\n");
		break;
	case A_M68K:
		DEBUG_LOG("Targeted CPU: M68K\n");
		break;
	case A_NS16K:
		DEBUG_LOG("Targeted CPU: NS16K\n");
		break;
	case A_SPARC:
		DEBUG_LOG("CPU ID: SPARC\n");
		break;
	default:
		DEBUG_LOG("CPU ID: Unknown\n");
		break;
	}

	ERROR_LOG("Unsupported CPU\n");
	destroy_env(&env);
	return -1;
}
