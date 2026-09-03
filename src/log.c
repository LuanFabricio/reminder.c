#include "log.h"

const char* log_cstr_label(const Log_Label label)
{
	switch (label) {
		case LOG_LABEL_INFO: return "INFO";
		case LOG_LABEL_WARNING: return "WARNING";
		case LOG_LABEL_ERROR: return "ERROR";
	}

	return "UNDEFINED";
}
