#ifndef _MACROS_H_
#define _MACROS_H_

#include <stdio.h>
#include <stdarg.h>

#include "callnr.h"
#include "com.h"
#include "const.h"
#include "utils.h"

#ifdef DEBUG
#define DEBUG_LOG(...)	debug_print(__VA_ARGS__)
#else
#define DEBUG_LOG(...)
#endif

#define ERROR_LOG(...)	fprintf(stderr, __VA_ARGS__)

#define MM_DEBUG_LOG(...) DEBUG_LOG("[MM] " __VA_ARGS__)
#define MM_ERROR_LOG(...) ERROR_LOG("[MM] " __VA_ARGS__)
#define FS_DEBUG_LOG(...) DEBUG_LOG("[FS] " __VA_ARGS__)
#define FS_ERROR_LOG(...) ERROR_LOG("[FS] " __VA_ARGS__)

#define ARRAY_DEFAULT_SIZE	8

#define TEXT_DATA_SEPERARED(env) ((env)->text != (env)->data)

#endif
