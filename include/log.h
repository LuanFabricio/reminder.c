#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

typedef enum {
	LOG_LABEL_INFO,
	LOG_LABEL_WARNING,
	LOG_LABEL_ERROR
} Log_Label;

const char* log_cstr_label(const Log_Label label);

#define log_format(file, label, ...)\
	do {\
		fprintf(file, "[%s|%s:%d]", log_cstr_label(label), __FILE__, __LINE__);\
		fprintf(file, __VA_ARGS__);\
	} while(0)

#define log_panic(...)\
	do {\
		log_format(stderr, LOG_LABEL_ERROR, __VA_ARGS__);\
		assert(false);\
	} while(0)
