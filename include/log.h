#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#define ANSI_COLOR_FG_BLUE 	"\x1b[34m"
#define ANSI_COLOR_FG_RED 	"\x1b[31m"
#define ANSI_COLOR_FG_YELLOW 	"\x1b[33m"
#define ANSI_COLOR_BG_BLUE 	"\x1b[37;44m"
#define ANSI_COLOR_BG_RED 	"\x1b[37;41m"
#define ANSI_COLOR_BG_YELLOW 	"\x1b[37;43m"
#define ANSI_COLOR_RESET 	"\x1b[0m"

typedef enum {
	LOG_LABEL_INFO,
	LOG_LABEL_WARNING,
	LOG_LABEL_ERROR
} Log_Label;

const char* log_cstr_label(const Log_Label label);
const char* log_cstr_label_fg_color(const Log_Label label);

#define log_format(file, label, ...)\
	do {\
		const char* FG_COLOR = log_cstr_label_fg_color(label);\
		fprintf(\
			file,\
			"%s["ANSI_COLOR_RESET\
			"%s"\
			"%s|%s:%s:%d]",\
			FG_COLOR,\
			log_cstr_label(label),\
			FG_COLOR,\
			__FILE__, __FUNCTION__, __LINE__\
		);\
		fprintf(file, __VA_ARGS__);\
		fprintf(file, ANSI_COLOR_RESET);\
	} while(0)

#define log_panic(...)\
	do {\
		log_format(stderr, LOG_LABEL_ERROR, __VA_ARGS__);\
		assert(false);\
	} while(0)
