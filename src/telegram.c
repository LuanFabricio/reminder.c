#include "telegram.h"

#include "env.h"
#include "scheduler.h"
#include "telebot/telebot-types.h"
#include "telebot/telebot.h"
#include <stdio.h>
#include <telebot/telebot-common.h>
#include <unistd.h>

static telebot_handler_t *handler = NULL;

void telegram__start()
{
	char* token = env_get_key("TELEGRAM_TOKEN");

	if (telebot_create(handler, token) != TELEBOT_ERROR_NONE) {
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
