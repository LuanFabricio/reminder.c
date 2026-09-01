
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <telebot/telebot-common.h>
#include <telebot/telebot-methods.h>
#include <telebot/telebot-types.h>
#include <unistd.h>

#include "telebot/telebot.h"

#include "env.h"
#include "scheduler.h"
#include "telegram.h"
#include "string_view.h"

static void telegram__setup_commands(telebot_handler_t handler)
{
	telebot_bot_command_t commands[] = {
		{"remind", "Adds a reminder."},
		{"info", "Get chat info"}
	};
	int commands_size = sizeof(commands) / sizeof(commands[0]);

	telebot_error_e error = telebot_set_my_commands(handler, commands, commands_size);
	if (error != TELEBOT_ERROR_NONE) {
		printf("Failed to setup commands (enum %d)\n", error);
	}
}

static void telegram__handle_message(telebot_handler_t handler, telebot_message_t* msg)
{
	if (msg->text == NULL) {
		return;
	}

	printf("Message from %s: %s\n", msg->from->first_name, msg->text);
	String_View sv = sv_from_cstr(msg->text);
	String_View_List svl = sv_split_n(sv, ' ', 2);

	char buffer[0xff];
	snprintf(buffer, sizeof(buffer), SV_FORMAT, SV_PRINT(svl.sv[1]));
	const uint32_t delay = (uint32_t)atoi(buffer);

	SchedulerMessage message = {
		.delay = delay,
		.type = MESSAGE_FLAG_TELEGRAM | MESSAGE_FLAG_EMAIL,
		.metadata = {
			.telegram = {
				.chat_id = msg->chat->id,
			},
			.email = {
				.subject = "Reminder",
			},
		},
	};

	strncpy(
		message.metadata.email.to,
		env_get_key("EMAIL_TO_ADDR"),
		sizeof(message.metadata.email.to));

	String_View sv_msg = svl.sv[svl.size-1];
	snprintf(message.message, sizeof(message.message), SV_FORMAT, SV_PRINT(sv_msg));

	scheduler_create(message);
}

static void telegram__start(telebot_handler_t* handler)
{
	char* token = env_get_key("TELEGRAM_TOKEN");

	telebot_error_e err = telebot_create(handler, token);
	if (err != TELEBOT_ERROR_NONE) {
		printf("Telebot create failed.\n");
		return;
	}
}

void telegram_send_message(const char* message, MessageMetadata data)
{
	static bool initialized = false;
	static telebot_handler_t handler;
	if (!initialized) {
		telegram__start(&handler);
		initialized = true;
	}

	MessageMetadataTelegram telegram_data = data.telegram;
	telebot_send_message(
		handler,
		telegram_data.chat_id,
		message, "", false, false, 0, "");
}

void *telegram_thread(void* ptr)
{
	telebot_handler_t handler;
	telegram__start(&handler);

	int offset = -1;

	telegram__setup_commands(handler);

	telebot_update_type_e update_types[] = {TELEBOT_UPDATE_TYPE_MESSAGE, TELEBOT_UPDATE_TYPE_CALLBACK_QUERY};
	int update_types_size = sizeof(update_types) / sizeof(update_types[0]);

	while(1) {
		int count = 0;
		telebot_update_t* updates = NULL;
		telebot_error_e error = telebot_get_updates(
			handler,
			offset, 20, 0,
			update_types, update_types_size,
			&updates, &count);

		if (error != TELEBOT_ERROR_NONE) {
			sleep(1);
			continue;
		}

		printf("Updates: %02d\n", count);
		for (uint32_t i = 0; i < count; i++) {
			telebot_update_t update = updates[i];
			if (update.update_type == TELEBOT_UPDATE_TYPE_MESSAGE) {
				telegram__handle_message(handler, &(update.message));
			}

			offset = update.update_id + 1;
		}

		telebot_put_updates(updates, count);
		sleep(1);
	}

	telebot_destroy(handler);
	handler = NULL;
}
