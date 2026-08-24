#pragma once

#include "map.h"
typedef Map Env;

void env_load(const char* filepath);
char* env_get_key(const char* key);
