#include <stdlib.h>
#include <string.h>

#include <ini.h>

#include <mmre.h>

static void
load_meta(const char *key, const char *value) {
	if (strcmp(key, "interval") == 0) {
		interval = atoi(value);
		assertf(interval, "Interval can't be zero.");
	} else {
		log(LOG_FAT, "Unknown meta key %s.", key);
	}
}

static struct user *user = NULL;

static void
load_user(const char *key, const char *value) {
	if (strcmp(key, "email") == 0) {
		log(LOG_DBG, "Reallocing users to %d.", nuser + 1);
		users = realloc(users, ++nuser * sizeof(struct user));
		user = &users[nuser - 1];
		memset(user, 0, sizeof(struct user));
		user->entries = malloc(sizeof(struct entry));

		user->email = strdup(value);
	} else if (user == NULL) {
		log(LOG_FAT, "The email property must be the first.");
	} else if (strcmp(key, "smtp_login") == 0) {
		assertf(!user->smtp_login, "Can't set smtp_login twice.");
		user->smtp_login = strdup(value);
	} else if (strcmp(key, "smtp_pwd") == 0) {
		assertf(!user->smtp_pwd, "Can't set smtp_pwd twice.");
		user->smtp_pwd = strdup(value);
	} else if (strcmp(key, "smtp_from") == 0) {
		assertf(!user->smtp_from, "Can't set smtp_from twice.");
		user->smtp_from = strdup(value);
	} else if (strcmp(key, "smtp_url") == 0) {
		assertf(!user->smtp_url, "Can't set smtp_url twice.");
		user->smtp_url = strdup(value);
	} else if (strcmp(key, "smtp_domain") == 0) {
		assertf(!user->smtp_domain, "Can't set smtp_domain twice.");
		user->smtp_domain = strdup(value);
	} else if (strcmp(key, "url") == 0) {
		user->entries = realloc(
		    user->entries, ++user->nentry * sizeof(struct entry)
		);
		struct entry *entry = &user->entries[user->nentry - 1];
		memset(entry, 0, sizeof(struct entry));

		entry->url = strdup(value);

		entry->nhash = load_hashes(&entry->hashes, user, value);
	} else {
		log(LOG_FAT, "Unknown user key %s.", key);
	}
}

static int
callback(void *user, const char *section, const char *key, const char *value) {
	if (strcmp(section, "meta") == 0) {
		load_meta(key, value);
	} else if (strcmp(section, "user") == 0) {
		load_user(key, value);
	}

	return 0;
}

static void
build_user(struct user *user) {
	for (int i=0; i < user->nentry; ++i)
		user->entries[i].owner = user;
}

static void
validate_user(struct user *user) {
#define CHECK_PROPERTY(user, prop)                                             \
	assertf(user->prop, "%s is not set for user %s", #prop, user->email);
	CHECK_PROPERTY(user, smtp_login);
	CHECK_PROPERTY(user, smtp_pwd);
	CHECK_PROPERTY(user, smtp_from);
	CHECK_PROPERTY(user, smtp_url);
	CHECK_PROPERTY(user, smtp_domain);

#undef CHECK_PROPERTY
}

void
load_config(char *path) {
	log(LOG_INF, "Reading configuration file %s.", path);
	// in case we are reloading at runtime
	for (int i = 0; i < nuser; ++i)
		user_free(&users[i]);
	free(users);
	nuser = 0;

	users = calloc(sizeof(struct user), 1);

	FILE *f = fopen(path, "r");
	assertf(f, "No configuration file found at %s", path);
	ini_parse_file(f, callback, NULL);
	fclose(f);
	
	for (int i = 0; i < nuser; ++i)
		build_user(&users[i]);

	for (int i = 0; i < nuser; ++i)
		validate_user(&users[i]);
	assertf(interval, "Interval was not set.");
}