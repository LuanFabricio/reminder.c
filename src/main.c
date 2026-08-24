#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "dynamic_array.h"
#include "env.h"
#include "map.h"
#include "string_view.h"
#include "scheduler.h"

da_create(int) int_list;

int main(int argc, char** argv)
{
	printf("Hello, world!\n");
	String_View sv = sv_from_cstr("Hello, world !");

	String_View_List svl = sv_split_n(sv, ' ', 2);
	for (int32_t i = 0; i < svl.size; i++) {
		printf(
			"[%02d|SV]"SV_FORMAT"\n",
			i+1, SV_PRINT(svl.sv[i]));
	}


	SchedulerMessage message = {
		.message = "Scheduler test!",
		.type = MESSAGE_FLAG_TELEGRAM | MESSAGE_FLAG_EMAIL,
		.delay = 1,
		.metadata = {0},
	};

	message.metadata.telegram.chat_id = 42;
	memcpy(message.metadata.email.to, "luan\0", 4);
	memcpy(message.metadata.email.subject, "nothing\0", 8);
	memcpy(message.metadata.email.body_template, "nothing 2\0", 10);
	scheduler_create(message);

	message.delay = 30;
	message.type = MESSAGE_FLAG_EMAIL;
	strncpy(message.message, "Scheduler test 2!", sizeof(message.message));
	scheduler_create(message);

	Map map = {0};
	map_add_node(&map, (Node){ .key = "key1", .value = "val1"});
	map_add_node(&map, (Node){ .key = "key2", .value = "val2"});

	for (uint32_t i = 0; i < map.size; i++) {
		Node n = map.items[i];
		printf("[%02u] %s: %s\n", i, n.key, n.value);
	}

	int32_t idx = map_get_node_index(&map, "asdf");

	env_load(".env");

	// asm("int3");

	for(;;) { sleep(5); }

	return 0;
}
