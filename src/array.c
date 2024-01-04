#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "array.h"
#include "macros.h"

Array array_create(size_t element_size)
{
	Array s;

	s.array = (void*) malloc(ARRAY_DEFAULT_SIZE * element_size);
	s.logical_size = 0;
	s.physical_size = ARRAY_DEFAULT_SIZE;
	s.element_size = element_size;

	assert(s.array);

	return s;
}

void array_free(Array* s)
{
	s->logical_size = 0;
	s->physical_size = 0;
	s->element_size = 0;

	if(s->array) {
		free(s->array);
		s->array = NULL;
	}
}

void array_set(Array* s, unsigned int index, void* elt_ptr)
{
	if(!elt_ptr)
		return;

	while(index >= s->logical_size)
		array_push(s, NULL);

	memcpy(s->array + (index * s->element_size), elt_ptr, s->element_size);
}

void array_push(Array* s, void* elt_ptr)
{
	if(s->physical_size == 0)
		return;

	if(s->logical_size >= s->physical_size)
	{
		s->physical_size = 2*s->logical_size;
		s->array = realloc(s->array, s->physical_size * s->element_size);
	}

	if(elt_ptr) {
		memcpy(s->array + (s->logical_size * s->element_size),
			elt_ptr, s->element_size);
	} else {
		bzero(s->array + (s->logical_size * s->element_size),
			s->element_size);
	}
	s->logical_size ++;
}

size_t array_size(Array* s)
{
	return s->logical_size;
}

void array_set_size(Array* s, size_t new_len)
{
	while(new_len > s->physical_size)
		array_push(s, NULL);

	s->logical_size = new_len;
}