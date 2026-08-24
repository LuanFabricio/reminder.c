#pragma once

#include <stdint.h>
#include <stdlib.h>

#define DA_MIN_CAPACITY 5

#define da_create(type)\
	typedef struct{\
		type * items;\
		int32_t size, capacity;\
	}

#define da_append(arr, x)\
	do {\
		if ((arr)->capacity <= 0 || (arr)->items == NULL) {\
			free((arr)->items);\
			(arr)->capacity = DA_MIN_CAPACITY;\
			(arr)->size = 0;\
			(arr)->items = malloc(sizeof(*(arr)->items) * (arr)->capacity);\
		}\
		else if (((arr)->size + 1) > (arr)->capacity) {\
			(arr)->capacity *= 2;\
			(arr)->items = realloc((arr)->items, sizeof(*(arr)->items) * (arr)->capacity);\
		}\
		(arr)->items[(arr)->size] = x;\
		(arr)->size += 1;\
	} while(0)
