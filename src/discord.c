#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "concord/discord.h"
#include "env.h"
#include "log.h"
#include "scheduler.h"
#include "string_view.h"

// NOTE: This global var with a function to initialize it
// can create some race conditions, maybe use a mutex or
// rewrite the handler usage.
static struct discord* handler = NULL;

void discord__start_handler()
{
	handler = discord_init(env_get_key("DISCORD_TOKEN"));
}

void discord_send_message(const char* message, MessageMetadata metadta)
{
	if (handler == NULL) {
		discord__start_handler();
	}

	struct discord_create_message params = {
		.content = (char*)message,
	};
	discord_create_message(handler, metadta.discord.chat_id, &params, NULL);
}

static void discord__on_ready(struct discord *client, const struct discord_ready *event)
{
	log_format(stdout, LOG_LABEL_INFO, "Discord thread ready!\n");
}

static void discord__on_remind(struct discord *handler, const struct discord_message *message)
{
	if (message->type != DISCORD_MESSAGE_DEFAULT) {
		return;
	}

	log_format(stdout, LOG_LABEL_INFO, "[%lu]MSG: %s\n", message->id, message->content);

	String_View sv = sv_from_cstr(message->content);
	String_View_List svl = sv_split_n(sv, ' ', 1);

	for (uint32_t i = 0; i < svl.size; i++) {
		log_format(
			stdout,
			LOG_LABEL_INFO,
			"[%02d] "SV_FORMAT"\n",
			i+1, SV_PRINT(svl.sv[i]));
	}

	if (sv_is_num(svl.sv[0])) {
		char buffer[0xff];
		snprintf(buffer, sizeof(buffer), SV_FORMAT, SV_PRINT(svl.sv[0]));
		const uint32_t delay = (uint32_t)atoi(buffer);

		SchedulerMessage scheduler_message = {
			.delay = delay,
			.type = MESSAGE_FLAG_TELEGRAM | MESSAGE_FLAG_DISCORD,
			.metadata = {
				.telegram = {
					.chat_id = atol(env_get_key("TELEGRAM_CHAT_ID"))
				},
				.discord = {
					.chat_id = message->channel_id,
				},
			}
		};
		snprintf(
			scheduler_message.message,
			sizeof(scheduler_message.message),
			SV_FORMAT,
			SV_PRINT(svl.sv[1]));

		log_format(
			stdout,
			LOG_LABEL_INFO,
			".delay=%u\n"
			".message=%s\n",
			scheduler_message.delay,
			scheduler_message.message);

		scheduler_create(scheduler_message);
	}
}

void *discord_thread(void *ptr)
{
	if (handler == NULL) {
		discord__start_handler();
	}

	discord_set_on_ready(handler, &discord__on_ready);

	discord_set_on_command(handler, "remind", &discord__on_remind);

	discord_run(handler);

	discord_cleanup(handler);

	return NULL;
}
