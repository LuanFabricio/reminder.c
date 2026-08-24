#include "env.h"
#include "map.h"
#include "string_view.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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
		printf("sv: "SV_FORMAT"\n", SV_PRINT(sv));
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

	for (int32_t i = 0; i < env.size; i++) {
		Node n = env.items[i];
		printf("[%02u] %s=%s \n", i, n.key, n.value);
	}
	int32_t idx = map_get_node_index(&env, "DISCORD_TOKEN");
	printf("idx: %i\n", idx);
	idx = map_get_node_index(&env, "EMAIL_PWD");
	printf("idx: %i\n", idx);
	idx = map_get_node_index(&env, "EMAIL_USERNAME");
	printf("idx: %i\n", idx);
}

char* env_get_key(const char* key)
{
	int32_t idx = map_get_node_index(&env, key);
	if (idx != -1) {
		return env.items[idx].value;
	}
	return NULL;
}
