#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "concord/discord.h"
#include "concord/discord_codecs.h"
#include "concord/concord-error.h"
#include "env.h"
#include "log.h"
#include "map.h"
#include "scheduler.h"
#include "string_view.h"

// NOTE: This global var with a function to initialize it
// can create some race conditions, maybe use a mutex or
// rewrite the handler usage.
static struct discord* handler = NULL;
u64snowflake g_app_id;

void discord__start_handler()
{
	handler = discord_init(env_get_key("DISCORD_TOKEN"));
	assert(handler != NULL && "Could not initialize client.");
}

void discord__setup_slash_commands(struct discord* client)
{
	struct discord_application_command_option_choice delay_units[] = {
		{
			.name = "second",
			.value = "1",
		},
		{
			.name = "minute",
			.value = "60",
		},
		{
			.name = "hour",
			.value = "3600",
		},
		{
			.name = "day",
			.value = "86400",
		},
	};

	struct discord_application_command_option options[] = {
		{
			.type = DISCORD_APPLICATION_OPTION_STRING,
			.name = "message",
			.description = "A message to remind.",
			.required = true,
		},
		{
			.type = DISCORD_APPLICATION_OPTION_INTEGER,
			.name = "delay",
			.description = "A delay to send the remind.",
			.required = true,
		},
		{
			.type = DISCORD_APPLICATION_OPTION_INTEGER,
			.name = "delay_unit",
			.description = "An unit to the delay",
			.required = true,
			.choices = &(struct discord_application_command_option_choices){
				.size = sizeof(delay_units) / sizeof(*delay_units),
				.array = delay_units,
			},
		},
	};

	struct discord_create_guild_application_command params = {
		.name = "remind",
		.description = "Create an remind to send after the delay.",
		.default_permission = true,
		.options = &(struct discord_application_command_options){
			.array = options,
			.size = sizeof(options) / sizeof(*options),
		},
	};

	const u64snowflake GUILD_ID = (u64snowflake)atol(env_get_key("DISCORD_GUILD_ID"));
	CCORDcode res = discord_create_guild_application_command(client, g_app_id, GUILD_ID, &params, NULL);
	log_format(
		stdout,
		LOG_LABEL_INFO,
		"create guild command result: %s(%i)\n",
		ccord_strerror(res),
		res);
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
	g_app_id = event->application->id;
	discord__setup_slash_commands(client);
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

		scheduler_create(scheduler_message, true);
	}
}

void discord__handle_remind_command(struct discord *client, const struct discord_interaction *event)
{
	struct discord_application_command_interaction_data_options *options = event->data->options;
	Map map = {0};
	if (options) {
		for (int i = 0; i < options->size; i++) {
			struct discord_application_command_interaction_data_option option = options->array[i];
			Node node = {0};
			strncpy(node.key, option.name, sizeof(node.key));
			strncpy(node.value, option.value, sizeof(node.value));
			map_add_node(&map, node);
		}
	}

	// NOTE: This assumes that `message`, `delay` and `delay_unit` are sent from discord.
	SchedulerMessage msg = {
		// TODO: Add option for channels to send.
		.type = MESSAGE_FLAG_DISCORD,
		.delay = (uint32_t)atoi(map_get_value(&map, "delay")) * (uint32_t)atoi(map_get_value(&map, "delay_unit")),
		.metadata = {
			0,
			.discord.chat_id = event->channel_id,
		}
	};
	strncpy(msg.message, map_get_value(&map, "message"), sizeof(msg.message));
	scheduler_create(msg, true);

	struct discord_interaction_response params = {
		.type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
		.data = &(struct discord_interaction_callback_data) {
			.content = "Remind created!",
		},
	};
	discord_create_interaction_response(handler, event->id, event->token, &params, NULL);
}

void discord__on_interaction_create(struct discord *client, const struct discord_interaction *event)
{
	if (event->type != DISCORD_INTERACTION_APPLICATION_COMMAND) {
		return;
	}

	log_format(stdout, LOG_LABEL_INFO, "Event name: %s\n", event->data->name);
	if (strcmp(event->data->name, "remind") == 0) {
		discord__handle_remind_command(client, event);
	}
}

void *discord_thread(void *ptr)
{
	if (handler == NULL) {
		discord__start_handler();
	}

	discord_set_on_ready(handler, &discord__on_ready);
	discord_set_on_command(handler, "remind", &discord__on_remind);
	discord_set_on_interaction_create(handler, discord__on_interaction_create);

	discord_run(handler);

	discord_cleanup(handler);

	return NULL;
}
