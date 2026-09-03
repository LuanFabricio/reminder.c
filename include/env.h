#pragma once

#include "string_view.h"
#include "map.h"
typedef Map Env;

void env_load(const char* filepath);
char* env_get_key(const char* key);
String_View_List env_get_keys();
