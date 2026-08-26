#pragma once

#include "scheduler.h"

void telegram_send_message(const char* message, MessageMetadata data);
void *telegram_thread(void* ptr);
