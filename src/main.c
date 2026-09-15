#include <libpq-fe.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#include "database.h"
#include "discord.h"
#include "env.h"
#include "scheduler.h"
#include "telegram.h"

int main(int argc, char** argv)
{
	env_load(".env");

	PGconn *conn = database_connect();
	database_setup(conn);

	database_fetch_pending_messages();

	pthread_t scheduler_thread;
	pthread_create(&scheduler_thread, NULL, scheduler_database_check, NULL);
	pthread_t telegram_pthread;
	pthread_create(&telegram_pthread, NULL, telegram_thread, NULL);
	pthread_t discord_pthread;
	pthread_create(&discord_pthread, NULL, discord_thread, NULL);

	for(;;) {}

	return 0;
}
