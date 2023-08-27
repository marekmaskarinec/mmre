#ifndef MMRE_H
#define MMRE_H

#include <stdint.h>
#include <stdio.h>

#include <curl/curl.h>

enum logLevel { LOG_DBG, LOG_INF, LOG_ERR, LOG_FAT };

void
_log(enum logLevel level, char *msg, ...);
#ifdef NDEBUG
#define log _log
#else
#define log(lvl, ...)                                                          \
	do {                                                                   \
		if (lvl >= min_log_level)                                      \
			fprintf(log_file, "%s:%d: ", __FILE__, __LINE__);      \
		_log(lvl, __VA_ARGS__);                                        \
	} while (0)
#endif

#define assertf(exp, ...)                                                      \
	do {                                                                   \
		if (!exp)                                                      \
			log(LOG_FAT, __VA_ARGS__);                             \
	} while (0)
#define assertr(ret, exp, ...)                                                 \
	do {                                                                   \
		if (!exp) {                                                    \
			log(LOG_ERR, __VA_ARGS__);                             \
			return ret;                                            \
		}                                                              \
	} while (0)
#define assertg(label, exp, ...)                                               \
	do {                                                                   \
		if (!exp) {                                                    \
			log(LOG_ERR, __VA_ARGS__);                             \
			goto label;                                            \
		}                                                              \
	} while (0)

struct feed {
	char *name;
	char *author;
};

struct post {
	struct feed *parent;
	char *title;
	char *link;
	char *date;
	char *content;
	uint64_t hash;
};

struct user;

struct entry {
	struct user *owner;
	char *url;
	uint64_t *hashes;
	int nhash;
};

struct user {
	char *email;
	struct entry *entries;
	int nentry;
};

char *
get_url(const char *url);
int
send_email_SMTP(
    const char *smtp, const char *subject, const char *domain, const char *from,
    const char *to, const char *user, const char *pwd, const char *msg
);
int
parse_RSS(
    const char *url, const char *text,
    void (*callback)(void *user, struct post *), void *user
);
void
entry_free(struct entry *entry);
void
user_free(struct user *user);
size_t
load_hashes(uint64_t **hashes, struct user *user, const char *url);
void
load_config(char *path);
uint64_t
djb2(const char *str);
int
hash_cmp(const void *a, const void *b);
void
check_entry(struct user *user, struct entry *entry);
void
save_hash(struct entry *entry, uint64_t hash);
int
asprintf(char **buf, const char *fmt, ...);

#ifndef MMRE_NO_GLOB
extern FILE *log_file;
extern int min_log_level;
extern uint32_t interval;
extern char *smtp_login;
extern char *smtp_pwd;
extern char *smtp_from;
extern char *smtp_url;
extern char *smtp_domain;
extern struct user *users;
extern int nuser;
#endif

#endif
