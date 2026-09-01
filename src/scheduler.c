#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "scheduler.h"
#include "telegram.h"
#include "email.h"


typedef void (*SchedulerSendCallback)(const char*, MessageMetadata);

static void scheduler__send_email(const char* message, MessageMetadata metadata)
{
	if (strlen(metadata.email.to) == 0) {
		fprintf(stderr, "E-Mail metadata is a null pointer");
		return;
	}

	fprintf(stdout, "E-Mail to %s with the message: \n\t%s\n", metadata.email.to, message);
	email_send(message, metadata);
}

static void scheduler__send_telegram(const char* message, MessageMetadata metadata)
{
	if (metadata.telegram.chat_id == 0) {
		fprintf(stderr, "E-Mail metadata is a null pointer");
		return;
	}

	fprintf(
		stdout,
		"Telegram message to %ld with the message: \n\t%s\n",
		metadata.telegram.chat_id, message);
	telegram_send_message(message, metadata);
}

static SchedulerSendCallback send_callbacks[] = {
	[MESSAGE_FLAG_INDEX_TELEGRAM] = scheduler__send_telegram,
	[MESSAGE_FLAG_INDEX_EMAIL] = scheduler__send_email,
};

static void *scheduler__thread(void* ptr)
{
	SchedulerMessage *message = ptr;

	sleep(message->delay);

	for (uint32_t i = 0; i < MESSAGE_FLAG_INDEX_LAST; i++) {
		uint32_t flag = 1 << i;

		if (flag & message->type) {
			send_callbacks[i](message->message, message->metadata);
		}
	}

	free(ptr);

	return NULL;
}

void scheduler_create(SchedulerMessage message)
{
	pthread_t thread;
	void* message_ptr = malloc(sizeof(message));
	memcpy(message_ptr, &message, sizeof(message));
	pthread_create(&thread, NULL, scheduler__thread, message_ptr);

	fprintf(stdout, "Message scheduled with %02u seconds of delay!\n", message.delay);
}
