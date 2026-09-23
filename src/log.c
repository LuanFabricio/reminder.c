#include "log.h"

const char* log_cstr_label(const Log_Label label)
{
	switch (label) {
		case LOG_LABEL_INFO: return ANSI_COLOR_BG_BLUE"INFO"ANSI_COLOR_RESET;
		case LOG_LABEL_WARNING: return ANSI_COLOR_BG_YELLOW"WARNING"ANSI_COLOR_RESET;
		case LOG_LABEL_ERROR: return ANSI_COLOR_BG_RED"ERROR"ANSI_COLOR_RESET;
	}

	return "UNDEFINED";
}

const char* log_cstr_label_fg_color(const Log_Label label)
{
	switch (label) {
		case LOG_LABEL_INFO: return ANSI_COLOR_FG_BLUE;
		case LOG_LABEL_WARNING: return ANSI_COLOR_FG_YELLOW;
		case LOG_LABEL_ERROR: return ANSI_COLOR_FG_RED;
	}

	return "";
}
