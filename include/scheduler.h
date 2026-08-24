#pragma once
#include <stdint.h>

typedef enum {
	MESSAGE_FLAG_INDEX_TELEGRAM = 0,
	MESSAGE_FLAG_INDEX_EMAIL,
	MESSAGE_FLAG_INDEX_LAST,
} MessageFlagsIndex;

typedef enum {
	MESSAGE_FLAG_TELEGRAM 	= 1 << MESSAGE_FLAG_INDEX_TELEGRAM,
	MESSAGE_FLAG_EMAIL 	= 1 << MESSAGE_FLAG_INDEX_EMAIL,
	MESSAGE_FLAG_LAST	= 1 << MESSAGE_FLAG_INDEX_LAST,
} MessageFlags;

#define MESSAGE_FLAG_TO_INDEX(x) (((x) >> 1) << 1)
#define MESSAGE_FLAG_LEN MESSAGE_FLAG_TO_INDEX(MESSAGE_FLAG_LAST)

typedef struct {
	char to[0xff];
	char subject[0xff];
	char body_template[0xff];
} MessageMetadataEmail;

typedef struct {
	int64_t chat_id;
} MessageMetadataTelegram;

typedef struct {
	MessageMetadataEmail email;
	MessageMetadataTelegram telegram;
} MessageMetadata;

typedef struct {
	char message[0xff];
	uint32_t delay;
	MessageFlags type;
	MessageMetadata metadata;
} SchedulerMessage;

void scheduler_create(SchedulerMessage message);
