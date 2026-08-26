#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "string_view.h"

String_View sv_from_cstr(const char* cstr)
{
	String_View sv = {
		.size = strlen(cstr),
	};
	sv.content = malloc(sizeof(char) * sv.size);
	memcpy(sv.content, cstr, sv.size);

	return sv;
}

uint32_t sv_count_char(const String_View sv, const char c)
{
	uint32_t count = 0;
	for (uint32_t i = 0; i < sv.size; i++) {
		if (sv.content[i] == c) {
			count++;
		}
	}
	return count;
}

static void sv__append_svl(const String_View sv, String_View_List *svl, const uint32_t i, const uint32_t last_c_index)
{

	String_View *current_sv = &svl->sv[svl->size];
	current_sv->size = i - last_c_index;
	current_sv->content = malloc(sizeof(char) * (current_sv->size + 1));
	memcpy(current_sv->content, sv.content + last_c_index, current_sv->size);

	svl->size++;
}

String_View_List sv_split_n(const String_View sv, const char c, uint32_t n)
{
	if (n <= 0) {
		n = UINT32_MAX;
	}

	uint32_t splits = sv_count_char(sv, c) + 1;
	if (splits > n) {
		splits = n + 1;
	}
	String_View_List svl = {0};
	svl.sv = malloc(sizeof(*svl.sv) * splits);

	uint32_t last_c_index = 0;
	uint32_t i = 0;
	do {
		if (sv.content[i] == c) {
			sv__append_svl(sv, &svl, i, last_c_index);
			last_c_index = i + 1;
		}

		i++;
	} while(i < sv.size && svl.size < splits - 1);
	sv__append_svl(sv, &svl, sv.size, last_c_index);

	return svl;
}

void svl_free(String_View_List* svl)
{
	for (uint32_t i = 0; i < svl->size; i++) {
		free(svl->sv[i].content);
	}
}

void sv_trim(String_View *sv)
{
	char *content = sv->content;
	uint32_t size = sv->size;
	for (int32_t i = 0; i < size; i++, content++, size--) {
		const char c = *content;
		if (c != '\n' && c != ' ') {
			break;
		}
	}

	for (uint32_t i = size-1; i >= 0; i--, size--){
		const char c = content[i];
		if (c != '\n' && c != ' ') {
			break;
		}
	}
	char* new_content = malloc(size);
	memcpy(new_content, content, size);

	free(sv->content);
	sv->content = new_content;
	sv->size = size;
}
