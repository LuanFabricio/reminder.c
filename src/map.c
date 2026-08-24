#include "map.h"
#include "dynamic_array.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

bool map_is_key_used(const Map* map, const char* key)
{
	for (size_t i = 0; i < map->size; i++) {
		if (strcmp(map->items[i].key, key) == 0) {
			return true;
		}
	}

	return false;
}

void map_add_node(Map *map, Node node)
{
	if (map_is_key_used(map, node.key)) {
		fprintf(
			stderr,
			"Key \"%s\" is already on map\n",
			node.key);
		return;
	}

	da_append(map, node);
}

int32_t map_get_node_index(const Map* map, const char *key)
{
	for (int32_t i = 0; i < map->size; i++) {
		if (strcmp(map->items[i].key, key) == 0) {
			return i;
		}
	}
	return -1;
}
