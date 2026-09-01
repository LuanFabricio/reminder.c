#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include <curl/easy.h>
#include <time.h>
#include <unistd.h>

#include "env.h"
#include "scheduler.h"

typedef struct {
	const char* message;
	MessageMetadata metadata;
	size_t bytes_read;
} Upload_Context;

static char* email__get_smtp_curl()
{
	const char* SMTP_SERVER = env_get_key("SMTP_SERVER");
	const char* SMTP_PORT = env_get_key("SMTP_PORT");

	const char format[] = "smtps://%s";
	const size_t buffer_size = sizeof(format) + strlen(SMTP_SERVER) + strlen(SMTP_PORT);
	char* buffer = malloc(buffer_size);
	snprintf(buffer, buffer_size, format, SMTP_SERVER);

	return buffer;
}

static char* email__get_time_header()
{
	time_t now = time(NULL);
	char buffer[0x42];
	strftime(
		buffer,
		sizeof(buffer),
		"%a, %d %b %G %H:%M:%S %z",
		localtime(&now)
	);

	const uint32_t time_fmt_len = strlen(buffer);
	char* time_fmt = malloc(time_fmt_len);
	memcpy(time_fmt, buffer, time_fmt_len);

	return time_fmt;
}

#define TEMPLATE "Date: %s\r\n"\
	"To: %s\r\n"\
	"From: %s\r\n"\
	"Message-ID: <%lu@reminderbot>\r\n"\
	"Subject: %s\r\n"\
	"\r\n"\
	"%s\r\n"

static size_t email__read_cb(char *ptr, size_t size, size_t nmemb, void *userp)
{
	Upload_Context* context = (Upload_Context*)userp;
	size_t room = size * nmemb;

	if (size == 0 || nmemb == 0 || room < 1) {
		return 0;
	}
	char email_from_buffer[0xff];
	snprintf(
		email_from_buffer,
		sizeof(email_from_buffer),
		"<%s> %s",
		env_get_key("EMAIL_FROM_ADDR"),
		env_get_key("EMAIL_FROM_MAIL"));

	char* time_fmt = email__get_time_header();
	printf("Date: %s\n", time_fmt);
	char message[0xffff];
	snprintf(
		message,
		room,
		TEMPLATE,
		time_fmt,
		context->metadata.email.to,
		// email_to_buffer,
		email_from_buffer,
		time(NULL),
		context->metadata.email.subject,
		context->message);
	size_t len = strlen(message);
	printf("message:\n%s", message);
	free(time_fmt);
	time_fmt = NULL;

	char* data = &message[context->bytes_read];
	len = strlen(data);
	if (room < len) {
		len = room;
	}
	memcpy(ptr, data, len);
	context->bytes_read = len;

	return len;
}

void email_send(const char* message, MessageMetadata metadata)
{
	Upload_Context context = {
		.message = message,
		.metadata = metadata,
		.bytes_read = 0,
	};

	CURLcode result = curl_global_init(CURL_GLOBAL_ALL);
	CURL* curl = curl_easy_init();

	if (!curl) {
		printf("ERROR: %s\n", curl_easy_strerror(result));
		return;
	}

	char* smtp_curl = email__get_smtp_curl();
	curl_easy_setopt(curl, CURLOPT_URL, smtp_curl);
	free(smtp_curl);
	smtp_curl = NULL;

	curl_easy_setopt(curl, CURLOPT_MAIL_FROM, env_get_key("EMAIL_FROM_ADDR"));
	curl_easy_setopt(curl, CURLOPT_USERNAME, env_get_key("EMAIL_FROM_ADDR"));
	curl_easy_setopt(curl, CURLOPT_PASSWORD, env_get_key("EMAIL_PWD"));

	struct curl_slist *recipients = NULL;
	recipients = curl_slist_append(recipients, metadata.email.to);
	curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

	curl_easy_setopt(curl, CURLOPT_READFUNCTION, email__read_cb);
	curl_easy_setopt(curl, CURLOPT_READDATA, &context);
	curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

	result = curl_easy_perform(curl);

	if(result != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(result));
	}

	curl_slist_free_all(recipients);

	curl_easy_cleanup(curl);

	curl_global_cleanup();
}
