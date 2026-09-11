#include <stdio.h>

#include <libpq-fe.h>
#include <string.h>

#include "env.h"
#include "log.h"
#include "scheduler.h"

#define TABLE_REMIND "remind"

void database_setup(PGconn* conn)
{
#define CREATE_TABLE_REMIND \
	"create table if not exists "TABLE_REMIND"("\
	"	message text,"\
	"	senders_flag int,"\
	"	email_to varchar(100),"\
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

	char email_to[100];
	if (message.type & MESSAGE_FLAG_EMAIL) {
		snprintf(email_to, sizeof(email_to), "%s", message.metadata.email.to);
	} else {
		strcpy(email_to, "NULL");
	}
#define INSERT_TEMPLATE \
	"insert into "TABLE_REMIND"("\
	"	message,"\
	"	senders_flag,"\
	"	email_to,"\
	"	telegram_chat_id,"\
	"	discord_chat_id,"\
	"	delay"\
	")"\
	"	values('%s', %d, '%s', %s, %s, %d);"

	char buffer[0xffff];
	snprintf(
		buffer,
		sizeof(buffer),
		INSERT_TEMPLATE,
		message.message,
		message.type,
		email_to,
		telegram_chat_id,
		discord_chat_id,
		message.delay
	);
	log_format(stdout, LOG_LABEL_INFO, "Query:\n%s\n", buffer);

	PGresult *res = PQexec(conn, buffer);
	PQprint(stdout, res, NULL);
}

PGconn* database_connect()
{
	char buffer[0xff];
	snprintf(buffer, sizeof(buffer), "postgresql://%s:%s@%s", env_get_key("DB_USERNAME"), env_get_key("DB_PWD"), env_get_key("DB_HOST"));
	return PQconnectdb(buffer);
}

void database_disconnect(PGconn* conn)
{
	PQfinish(conn);
}
