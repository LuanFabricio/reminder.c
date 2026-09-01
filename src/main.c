#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dynamic_array.h"
#include "email.h"
#include "env.h"
#include "scheduler.h"
#include "telegram.h"

da_create(int) int_list;

int main(int argc, char** argv)
{
	env_load(".env");

	pthread_t telegram_pthread;
	pthread_create(&telegram_pthread, NULL, telegram_thread, NULL);

	// asm("int3");

	for(;;) {}

	return 0;
}
