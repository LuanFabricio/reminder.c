
#include <stdio.h>
#include <stdlib.h>
#include <telebot/telebot-common.h>
#include <unistd.h>

#include "telebot/telebot.h"

#include "env.h"
#include "scheduler.h"
#include "telegram.h"

static telebot_handler_t *handler = NULL;

void telegram__start()
{
	handler = malloc(sizeof(*handler));
	char* token = env_get_key("TELEGRAM_TOKEN");

	telebot_error_e err = telebot_create(handler, token);
	if (err != TELEBOT_ERROR_NONE) {
		printf("Telebot create failed.\n");
		return;
	}
}

void telegram_send_message(const char* message, MessageMetadata data)
{
	if (!handler) {
		telegram__start();
	}

	MessageMetadataTelegram telegram_data = data.telegram;
	telebot_send_message(
		*handler,
		telegram_data.chat_id,
		message, "", false, false, 0, "");
}
