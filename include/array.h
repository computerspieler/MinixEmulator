#ifndef _ARRAY_H_
#define _ARRAY_H_

#include <stddef.h>

typedef struct {
	void *array;
	
	size_t physical_size;
	size_t logical_size;
	size_t element_size;
} Array;

Array array_create(size_t element_size);
void array_free(Array*);
void array_set(Array*, unsigned int, void*);
void array_push(Array*, void*);
size_t array_size(Array*);
void array_set_size(Array*, size_t);
void array_clear(Array*);
void array_get(Array*, int, void*);

#endif
