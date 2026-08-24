#include <curl/curl.h>
#include <curl/easy.h>

// TODO: Add server and port
#define SMTP_SERVER ""
#define SMTP_PORT ":"

void email_send()
{
	CURL* curl = curl_easy_init();

	curl_easy_setopt(curl, CURLOPT_URL, "smpt://"SMTP_SERVER SMTP_PORT);
}
