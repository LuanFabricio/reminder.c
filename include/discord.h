#pragma once

#include "scheduler.h"
void discord_send_message(const char* message, MessageMetadata metadta);
void *discord_thread(void *ptr);
