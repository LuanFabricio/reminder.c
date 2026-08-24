#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "dynamic_array.h"

#define NODE_KEY_BUFFER_SIZE 0xff
#define NODE_VALUE_BUFFER_SIZE 0xff

typedef struct {
	char key[NODE_KEY_BUFFER_SIZE];
	char value[NODE_VALUE_BUFFER_SIZE];
} Node;

da_create(Node) Map;

bool map_is_key_used(const Map* map, const char* key);
void map_add_node(Map* map, Node node);
int map_get_node_index(const Map* map, const char *key);
