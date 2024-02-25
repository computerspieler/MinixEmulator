#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/times.h>

#include "services.h"
#include "macros.h"
#include "fs.h"
#include "fs_stat.h"
#include "utils.h"
#include "fs_param.h"

#define WRITE_IF_POSSIBLE(base_address, index, max_index, data)	\
	if((index) < (max_index)) {	\
		env->write_byte(env, (base_address) + (index), (data));	\
	}

int fs_interpret_message(Emulator_Env *env, uint32_t dest_src, message *mess, int direction)
{
	int i, j;
	char c;
	int flags, cmd;
	int ret;
	char *path;
	FileHandler file_handler;
	struct dirent *dir_ent;
	int dir_ent_name_length;
	struct tms tms_buf;

	FS_DEBUG_LOG("Message type: %s(%d)\n",
		callnr_to_string[mess->m_type], mess->m_type);

	switch(mess->m_type) {
	case IOCTL:
		FS_DEBUG_LOG("Line %x;\n", mess->TTY_LINE);
		
		env->response.m_type   = DEV_IOCTL;
		env->response.DEVICE   = (mess->TTY_LINE >> MINOR) & 0xFF;
		env->response.PROC_NR  = mess->m_source;
		env->response.ADDRESS  = mess->ADDRESS;
		env->response.POSITION = 0;
		env->response.COUNT    = mess->TTY_REQUEST;
		return 0;
	
	case WRITE:
#ifdef DEBUG
		// Send everything to stderr, it's
		// easier to debug
		if(mess->fd == STDOUT_FILENO)
			mess->fd = STDERR_FILENO;
#endif
		array_get(&env->file_handlers, mess->fd, &file_handler);
		if(!file_handler.has_stat)
			return -1;

		// We don't support writing to directory
		// for now
		if(file_handler.dir_p)
			return -1;

		for(i = 0; i < mess->nbytes; i ++) {
			c = env->read_byte(env, mess->buffer+i);
			if(!write(file_handler.file_d, &c, sizeof(char)))
				break;
		}
		
		env->response.m_type = 0;
		env->response.PROC_NR = mess->m_source;
		return i;
	
	case READ:
		array_get(&env->file_handlers, mess->fd, &file_handler);
		
		if(file_handler.dir_p) {
			i = 0;
			while((dir_ent = readdir(file_handler.dir_p)) != NULL) {
				/*
					This is the structure used for a
					directory's entry :
					struct _v7_direct {		
						unsigned short	d_ino;
						char			d_name[14];
					};
				*/
				WRITE_IF_POSSIBLE(mess->buffer, i, mess->nbytes,
					dir_ent->d_ino & 0xFF);
				i ++;
				WRITE_IF_POSSIBLE(mess->buffer, i, mess->nbytes,
					(dir_ent->d_ino >> 8) & 0xFF);
				i ++;

				dir_ent_name_length = strlen(dir_ent->d_name);
				for(j = 0; j < MIN(dir_ent_name_length, 14); j ++, i ++) {
					WRITE_IF_POSSIBLE(mess->buffer, i, mess->nbytes,
						dir_ent->d_name[j]);
				}
				for(; j < 14; j ++, i ++) {
					WRITE_IF_POSSIBLE(mess->buffer, i, mess->nbytes, 0);
				}
				
				if(i >= mess->nbytes)
					break;
			}
		} else {
			for(i = 0; i < mess->nbytes; i ++) {
				if(!read(file_handler.file_d, &c, sizeof(char)))
					break;
				env->write_byte(env, mess->buffer+i, c);
			}

			env->response.m_type = 0;
			env->response.PROC_NR = mess->m_source;
		}

		return i;

	case TIME:
		env->response.reply_l1 = (cpu_ptr) time(NULL);
		return 0;

	case CLOSE:
		array_get(&env->file_handlers, mess->fd, &file_handler);
		
		if(file_handler.dir_p)
			ret = closedir(file_handler.dir_p);
		else
			ret = close(file_handler.file_d);

		bzero(&file_handler, sizeof(FileHandler));
		array_set(&env->file_handlers, mess->fd, &file_handler);
		return ret;

	case OPEN:
		flags = 0;

		if(mess->mode & FS_O_CREAT)
			path = get_path(env, mess->buffer, mess->name1_length);
		else
			path = get_path(env, mess->name, mess->name_length);

		if(!path)
			return -1;

		if(mess->mode & FS_O_EXCL) flags |= O_EXCL;
		if(mess->mode & FS_O_CREAT) flags |= O_CREAT;
		if(mess->mode & FS_O_TRUNC) flags |= O_TRUNC;
		if(mess->mode & FS_O_NOCTTY) flags |= O_NOCTTY;

		if(mess->mode & FS_O_APPEND) flags |= O_APPEND;
		if(mess->mode & FS_O_NONBLOCK) flags |= O_NONBLOCK;

		if(mess->mode & FS_O_RDONLY) flags |= O_RDONLY;
		if(mess->mode & FS_O_WRONLY) flags |= O_WRONLY;
		if(mess->mode & FS_O_RDWR)   flags |= O_RDWR;
		
		if(mess->mode & FS_O_CREAT)
			printf("Create a new file !\n");

		if(access(path, F_OK) == 0)
			ret = get_stat_from_path(path, &file_handler.statbuf);
		else
			ret = 0;

		if(!ret && FS_S_ISDIR(file_handler.statbuf.s_mode)) {
			file_handler.dir_p = opendir(path);
			file_handler.file_d = -1;
			file_handler.has_stat = 1;
			free(path);
			if(!file_handler.dir_p)
				return -1;
		} else {
			file_handler.file_d = open(path, flags);
			file_handler.dir_p = NULL;
			free(path);
			if(file_handler.dir_p)
				return -1;
			
			if(!ret)
				file_handler.has_stat =
					get_stat(file_handler.file_d, &file_handler.statbuf);
			else
				file_handler.has_stat = 1;
		}
		
		ret = array_size(&env->file_handlers);
		array_push(&env->file_handlers, &file_handler);

		return ret;

	case FCNTL:
		array_get(&env->file_handlers, mess->fd, &file_handler);

		switch(mess->request) {
		case FS_F_DUPFD:  cmd = F_DUPFD;  break;
		case FS_F_GETFD:  cmd = F_GETFD;  break;
		case FS_F_SETFD:  cmd = F_SETFD;  break;
		case FS_F_GETFL:  cmd = F_GETFL;  break;
		case FS_F_SETFL:  cmd = F_SETFL;  break;
		case FS_F_GETLK:  cmd = F_GETLK;  break;
		case FS_F_SETLK:  cmd = F_SETLK;  break;
		case FS_F_SETLKW: cmd = F_SETLKW; break;
		}

		// FIXME: Don't ignore fcntl
		// for directories
		if(file_handler.file_d >= 0)
			return fcntl(file_handler.file_d, cmd);
		else
			return 0;

	case MKDIR:
		path = get_path(env, mess->buffer, mess->name1_length);
		if(!path)
			return -1;

		ret = mkdir(path, mess->mode);
		free(path);

		return ret;

	case RMDIR:
		path = get_path(env, mess->name, mess->name_length);
		if(!path)
			return -1;

		ret = rmdir(path);
		free(path);

		return ret;

	case UMASK:
		return umask(mess->co_mode);

	case UNLINK:
		path = get_path(env, mess->name, mess->name_length);
		if(!path)
			return -1;

		ret = unlink(path);
		free(path);

		return ret;
	
	case LSEEK:
		array_get(&env->file_handlers, mess->ls_fd, &file_handler);
		return lseek(file_handler.file_d, mess->offset, mess->whence);

	case ACCESS:
		path = get_path(env, mess->name, mess->name_length);
		if(!path)
			return -1;
		ret = access(path, mess->m3_i2);
		free(path);

		return ret;

	case STAT:
		path = get_path(env, mess->buffer, mess->name1_length);
		if(!path)
			return -1;

		ret = get_stat_from_path(path, &file_handler.statbuf);
		free(path);

		if(!ret) {
			for(i = 0; i < (int) sizeof(struct fs_stat); i ++)
				env->write_byte(env, mess->m1_p2+i, ((uint8_t*)&file_handler.statbuf)[i]);
		}

		return ret;

	case FSTAT:
		array_get(&env->file_handlers, mess->fd, &file_handler);
		if(!file_handler.has_stat)
			return -1;

		for(i = 0; i < (int) sizeof(struct fs_stat); i ++) {
			env->write_byte(env, mess->m1_p1+i,
				((uint8_t*)&file_handler.statbuf)[i]);
		}

		return 0;

	case TIMES:
		ret = times(&tms_buf);
		mess->m4_l1 = tms_buf.tms_utime;
		mess->m4_l2 = tms_buf.tms_stime;
		mess->m4_l3 = tms_buf.tms_cutime;
		mess->m4_l4 = tms_buf.tms_cstime;
		return ret;

	default:
		FS_ERROR_LOG("Unknown message type : %d\n", mess->m_type);
		return 1;
	}
}
