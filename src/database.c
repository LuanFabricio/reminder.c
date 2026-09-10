#include <stdio.h>
#include <stdlib.h>

#include <libpq-fe.h>
#include <string.h>

#include "env.h"
#include "log.h"
#include "scheduler.h"

#define TABLE_REMIND "remind"

void database__setup(PGconn* conn)
{
#define CREATE_TABLE_REMIND \
	"create table if not exists "TABLE_REMIND"("\
	"	message text,"\
	"	senders_flag int,"\
	"	telegram_chat_id bigint,"\
	"	discord_chat_id bigint,"\
	"	delay int,"\
	"	created_at timestamp default current_timestamp"\
	");"
	log_format(stdout, LOG_LABEL_INFO, "Setup: %s\n", CREATE_TABLE_REMIND);
	PGresult* res = PQexec(conn, CREATE_TABLE_REMIND);
	PQprint(stdout, res, NULL);
#undef CREATE_TABLE_REMIND
}

void database_insert_remind(PGconn* conn, SchedulerMessage message)
{
	char telegram_chat_id[0xff];
	if (message.type & MESSAGE_FLAG_TELEGRAM) {
		snprintf(
			telegram_chat_id,
			sizeof(telegram_chat_id),
			"%li", message.metadata.telegram.chat_id);
	} else {
		strcpy(telegram_chat_id, "NULL");
	}

	char discord_chat_id[0xff];
	if (message.type & MESSAGE_FLAG_DISCORD) {
		snprintf(
			discord_chat_id,
			sizeof(discord_chat_id),
			"%lu", message.metadata.discord.chat_id);
	} else {
		strcpy(discord_chat_id, "NULL");
	}
#define INSERT_TEMPLATE \
	"insert into "TABLE_REMIND"("\
	"	message,"\
	"	senders_flag,"\
	"	telegram_chat_id,"\
	"	discord_chat_id,"\
	"	delay"\
	")"\
	"	values('%s', %d, %s, %s, %d);"

	char buffer[0xffff];
	snprintf(
		buffer,
		sizeof(buffer),
		INSERT_TEMPLATE,
		message.message,
		message.type,
		telegram_chat_id,
		discord_chat_id,
		message.delay
	);
	log_format(stdout, LOG_LABEL_INFO, "Query:\n%s\n", buffer);

	PGresult *res = PQexec(conn, buffer);
	PQprint(stdout, res, NULL);
}

void database_connect()
{
	// PGconn *conn = PQconnectdbParams(const char *const *keywords, const char *const *values, int expand_dbname);
	char buffer[0xff];
	snprintf(buffer, sizeof(buffer), "postgresql://%s:%s@localhost", env_get_key("DB_USERNAME"), env_get_key("DB_PWD"));
	PGconn *conn = PQconnectdb(buffer);

	database__setup(conn);

	SchedulerMessage msg = {
		.message = "some message\nteste\n",
		.type = MESSAGE_FLAG_DISCORD | MESSAGE_FLAG_TELEGRAM,
		.metadata = {
			.telegram = {
				.chat_id = 1
			},
			.discord = {
				.chat_id = 2
			},
		},
		.delay = 32,
	};
	database_insert_remind(conn, msg);

	int ret = PQsendQuery(conn, "select 1 as n;");
	if (ret != 1) {
		exit(1);
	}

	PGresult *res = NULL;

	while ((res = PQgetResult(conn)) != NULL) {
		PQprintTuples(res, stdout, 1, 0, 0);
	}

	ret = PQsendQuery(conn, "select * from remind;");
	if (ret != 1) {
		log_panic("Invalid return %s(%d)\n", PQerrorMessage(conn), ret);
	}
	while ((res = PQgetResult(conn)) != NULL) {
		PQprintTuples(res, stdout, 1, 0, 0);
	}

	PQfinish(conn);
}
