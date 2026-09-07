#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#include "dynamic_array.h"
#include "discord.h"
#include "env.h"
#include "telegram.h"

da_create(int) int_list;

int main(int argc, char** argv)
{
	env_load(".env");

	pthread_t telegram_pthread;
	pthread_create(&telegram_pthread, NULL, telegram_thread, NULL);
	pthread_t discord_pthread;
	pthread_create(&discord_pthread, NULL, discord_thread, NULL);

	// asm("int3");

	for(;;) {}

	return 0;
}
