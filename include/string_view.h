#pragma once

#include <stdbool.h>
#include <stdint.h>

#define SV_FORMAT "%.*s"
#define SV_PRINT(sv) (sv).size, (sv).content

typedef struct {
	char* content;
	uint32_t size;
} String_View;

typedef struct {
	String_View *sv;
	uint32_t size;
} String_View_List;

String_View sv_from_cstr(const char* cstr);
uint32_t sv_count_char(const String_View sv, const char c);
String_View_List sv_split_n(const String_View sv, const char c, uint32_t n);
void svl_free(String_View_List* svl);
void sv_trim(String_View *sv);
