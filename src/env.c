#include "env.h"
#include "map.h"
#include "string_view.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static Env env = {0};

void env_load(const char *filepath)
{
	FILE* file = fopen(filepath, "r");
	if (file == NULL) {
		fprintf(stderr, "Error: Make sure that `%s` exists\n", filepath);
		exit(1);
	}

	size_t buffer_size = 0xff;
	char* buffer = malloc(buffer_size);
	while(getline(&buffer, &buffer_size, file) > 1) {
		String_View sv = sv_from_cstr(buffer);
		sv_trim(&sv);
		String_View_List svl = sv_split_n(sv, '=', 1);

		assert(svl.size == 2);

		Node n = {0};
		snprintf(n.key, NODE_KEY_BUFFER_SIZE, SV_FORMAT, SV_PRINT(svl.sv[0]));
		snprintf(n.value, NODE_VALUE_BUFFER_SIZE, SV_FORMAT, SV_PRINT(svl.sv[1]));
		map_add_node(&env, n);

		free(sv.content);
		svl_free(&svl);

	}
	free(buffer);
	fclose(file);
}

char* env_get_key(const char* key)
{
	int32_t idx = map_get_node_index(&env, key);
	if (idx != -1) {
		return env.items[idx].value;
	}
	return NULL;
}

String_View_List env_get_keys()
{
	String_View_List svl = {0};
	svl.size = env.size;
	svl.sv = malloc(sizeof(*svl.sv));

	for (uint32_t i = 0; i < env.size; i++) {
		const char* key = env.items[i].key;
		String_View *sv = &(svl.sv[i]);
		sv->size = strlen(key);
		sv->content = malloc(sv->size);
		memcpy(sv->content, key, sv->size);
	}

	return svl;
}
