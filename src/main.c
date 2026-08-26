#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dynamic_array.h"
#include "env.h"
#include "scheduler.h"
#include "telegram.h"

da_create(int) int_list;

int main(int argc, char** argv)
{
	env_load(".env");

	SchedulerMessage message = {
		.message = "Scheduler test!",
		.type = MESSAGE_FLAG_TELEGRAM | MESSAGE_FLAG_EMAIL,
		.delay = 1,
		.metadata = {0},
	};

	const uint64_t chat_id = atoll(env_get_key("TELEGRAM_CHAT_ID"));

	message.metadata.telegram.chat_id = chat_id;
	memcpy(message.metadata.email.to, "luan\0", 4);
	memcpy(message.metadata.email.subject, "nothing\0", 8);
	memcpy(message.metadata.email.body_template, "nothing 2\0", 10);
	scheduler_create(message);

	message.delay = 30;
	message.type = MESSAGE_FLAG_TELEGRAM;
	message.metadata.telegram.chat_id = chat_id;
	strncpy(message.message, "Scheduler test 2!", sizeof(message.message));
	scheduler_create(message);

	pthread_t telegram_pthread;
	pthread_create(&telegram_pthread, NULL, telegram_thread, NULL);

	// asm("int3");

	for(;;) {}

	return 0;
}
