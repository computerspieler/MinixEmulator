#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>
#include <unistd.h>

#include "type.h"
#include "macros.h"
#include "i386.h"

char *get_path(Emulator_Env *env, uint32_t path_ptr, int length)
{
	int i;
	int off;
	int absolute;
	char *buf;

	if(env->read_byte(env, path_ptr) == '/') {
		absolute = 1;
		off = env->chroot_path_length;
	} else {
		absolute = 0;
		off = 0;
	}

	buf = malloc(sizeof(char) * (length + off));
	if(!buf)
		return NULL;

	if(absolute)
		memcpy(buf, env->chroot_path, env->chroot_path_length);

	for(i = 0; i < length; i ++) 
		buf[i+off] = env->read_byte(env, path_ptr+i);

#ifdef DEBUG
	DEBUG_LOG("Path: %s\n", buf);
#endif

	return buf;
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

int get_stat_from_path(char *path, struct fs_stat *out_statbuf)
{
	struct stat statbuf;

	assert(path);
	assert(out_statbuf);

	if(stat(path, &statbuf)) {
		perror("stat");
		return 1;
	}

	out_statbuf->s_dev   = statbuf.st_dev;
	out_statbuf->s_ino   = statbuf.st_ino;
	out_statbuf->s_mode  = statbuf.st_mode;
	out_statbuf->s_nlink = statbuf.st_nlink;
	out_statbuf->s_uid   = statbuf.st_uid;
	out_statbuf->s_gid   = statbuf.st_gid;
	out_statbuf->s_rdev  = statbuf.st_rdev;
	out_statbuf->s_size  = statbuf.st_size;
	out_statbuf->s_atime = statbuf.st_atim.tv_sec;
	out_statbuf->s_mtime = statbuf.st_mtim.tv_sec;
	out_statbuf->s_ctime = statbuf.st_ctim.tv_sec;

	return 0;
}

int get_stat(int fd, struct fs_stat *ret)
{
	struct stat statbuf;

	assert(fd >= 0);

	if(fstat(fd, &statbuf))
		return 0;

	ret->s_dev   = statbuf.st_dev;
	ret->s_ino   = statbuf.st_ino;
	ret->s_mode  = statbuf.st_mode;
	ret->s_nlink = statbuf.st_nlink;
	ret->s_uid   = statbuf.st_uid;
	ret->s_gid   = statbuf.st_gid;
	ret->s_rdev  = statbuf.st_rdev;
	ret->s_size  = statbuf.st_size;
	ret->s_atime = statbuf.st_atim.tv_sec;
	ret->s_mtime = statbuf.st_mtim.tv_sec;
	ret->s_ctime = statbuf.st_ctim.tv_sec;

	return 1;
}

int build_env(Emulator_Env *env)
{
	FileHandler tmp_handler;

	bzero(env, sizeof(Emulator_Env));
	
	env->stack = array_create(sizeof(char));
	env->heap = array_create(sizeof(char));
	env->symbols = array_create(sizeof(struct nlist));
	env->file_handlers = array_create(sizeof(FileHandler));

	tmp_handler.dir_p = NULL;
	// Add the default streams
	tmp_handler.file_d = STDIN_FILENO;
	tmp_handler.has_stat = get_stat(STDIN_FILENO, &tmp_handler.statbuf);
	array_set(&env->file_handlers, 0, &tmp_handler);
	
	tmp_handler.file_d = STDOUT_FILENO;
	tmp_handler.has_stat = get_stat(STDOUT_FILENO, &tmp_handler.statbuf);
	array_set(&env->file_handlers, 1, &tmp_handler);
	
	tmp_handler.file_d = STDERR_FILENO;
	tmp_handler.has_stat = get_stat(STDERR_FILENO, &tmp_handler.statbuf);
	array_set(&env->file_handlers, 2, &tmp_handler);
	
	return 0;
}

void destroy_env(Emulator_Env *env)
{
	int i;
	FileHandler* fh;

	fh = env->file_handlers.array;
	for(i = 0; i < (int) array_size(&env->file_handlers); i ++) {
		if(FS_S_ISDIR(fh[i].statbuf.s_mode))
			closedir(fh[i].dir_p);
		else
			close(fh[i].file_d);
	}

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

int run_executable(Emulator_Env *env)
{
	int return_code;

	switch(env->hdr.a_cpu) {
	case A_I80386:
		return_code = run_x86_emulator(env);
		destroy_env(env);
		return return_code;
	case A_I8086:
	case A_M68K:
	case A_NS16K:
	case A_SPARC:
	default:
		break;
	}

	destroy_env(env);
	return -1;
}

void add_arguments_env(Emulator_Env *env, int argc, char* argv[])
{
	switch(env->hdr.a_cpu) {
	case A_I80386:
		DEBUG_LOG("CPU ID: I386\n");
		x86_emulator_init(env, argc, argv);
		return;
	case A_I8086:
		DEBUG_LOG("Targeted CPU: I86\n");
		return;
	case A_M68K:
		DEBUG_LOG("Targeted CPU: M68K\n");
		return;
	case A_NS16K:
		DEBUG_LOG("Targeted CPU: NS16K\n");
		return;
	case A_SPARC:
		DEBUG_LOG("CPU ID: SPARC\n");
		return;
	default:
		DEBUG_LOG("CPU ID: Unknown\n");
		return;
	}
}