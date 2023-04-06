#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <mmre.h>

struct wbuf {
	char *str;
	int len;
	int cap;
};

static size_t
getUrl__writeFunction(char *ptr, size_t size, size_t nmemb, struct wbuf *b) {
	if (b->len + nmemb < b->cap) {
		memcpy(b->str + b->len, ptr, nmemb);
		b->len += nmemb;
		return nmemb;
	}

	if (b->len + nmemb < b->cap * 2) {
		b->str = realloc(b->str, b->cap *= 2);
		memcpy(b->str + b->len, ptr, nmemb);
		b->len += nmemb;
		return nmemb;
	}

	b->cap = (b->len + nmemb) * 2;
	b->str = realloc(b->str, b->cap);
	memcpy(b->str + b->len, ptr, nmemb);
	b->len += nmemb;

	return nmemb;
}

char *
get_url(const char *url) {
	log(LOG_DBG, "Getting %s.", url);
	CURL *curl = curl_easy_init();
	assertr(NULL, curl, "Could not init curl.");

	struct wbuf b =
	    (struct wbuf){.str = calloc(32, sizeof(char)), .cap = 32};

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(
	    curl, CURLOPT_USERAGENT,
	    "mmre v0.1 - RSS feed to email daemon https://mrms.cz/mmre"
	);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, getUrl__writeFunction);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &b);

	assertg(
	    fail, !curl_easy_perform(curl), "Could not perform request to %s",
	    url
	);

	b.str[b.len] = 0;

	curl_easy_cleanup(curl);

	return b.str;

fail:
	free(b.str);
	curl_easy_cleanup(curl);
	return NULL;
}

struct rbuf {
	size_t idx;
	size_t len;
	char *buf;
};

static size_t
sendEmailSMTP__readFunction(
    char *ptr, size_t size, size_t nmemb, struct rbuf *b
) {
	if (b->idx + nmemb >= b->len) {
		nmemb = b->len - b->idx;
	}

	memcpy(ptr, b->buf + b->idx, nmemb);
	b->idx += nmemb;
	return nmemb;
}

void
send_email_SMTP(
    const char *smtp, const char *subject, const char *domain, const char *from,
    const char *to, const char *user, const char *pwd, const char *msg
) {
	log(LOG_DBG, "Sending email to %s.", to);
	CURL *curl = curl_easy_init();
	assertr(, curl, "Could not init curl.");

	curl_easy_setopt(curl, CURLOPT_URL, smtp);
	curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_TRY);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

	curl_easy_setopt(curl, CURLOPT_MAIL_FROM, from);

	struct curl_slist *rcpt = NULL;
	rcpt = curl_slist_append(rcpt, to);
	curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, rcpt);

	curl_easy_setopt(curl, CURLOPT_USERNAME, user);
	curl_easy_setopt(curl, CURLOPT_PASSWORD, pwd);

	srand(time(NULL));

	const time_t t = time(NULL);
	const struct tm *tm = localtime(&t);

	const char *weekdays[] = {"Sun", "Tue", "Wed", "Thu",
				  "Fri", "Sat", "Mon"};
	const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
				"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

	struct rbuf buf = (struct rbuf){
	    .len = strlen(msg),
	};

	asprintf(
	    &buf.buf,
	    "Date: %s, %d %s %d %d:%d:%d +0100\r\n"
	    "To: %s\r\n"
	    "From: %s\r\n"
	    "User-Agent: mmre v0.1\r\n"
	    "Message-ID: <%x-%x-%x-%x-%x@%s>\r\n"
	    "Subject: %s\r\n"
	    "\r\n"
	    "%s\r\n",
	    weekdays[tm->tm_wday], tm->tm_mday, months[tm->tm_mon],
	    tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec, to, from,
	    rand(), rand(), rand(), rand(), rand(), domain, subject, msg
	);
	buf.len = strlen(buf.buf);

	curl_easy_setopt(
	    curl, CURLOPT_READFUNCTION, sendEmailSMTP__readFunction
	);
	curl_easy_setopt(curl, CURLOPT_READDATA, &buf);
	curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

	assertg(
	    cleanup, !curl_easy_perform(curl), "Could not send email to %s.", to
	);

cleanup:
	curl_slist_free_all(rcpt);
	free(buf.buf);

	curl_easy_cleanup(curl);
}