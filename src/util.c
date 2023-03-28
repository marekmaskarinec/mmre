#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include <mmre.h>

uint64_t
djb2(const char *str)
{
	uint64_t hash = 5381;
	int c;

	while ((c = *str++)) {
		hash = ((hash << 5) + hash) + c;
	}

	return hash;
}

void
_log(enum logLevel level, char *msg, ...)
{
	if (level < min_log_level)
		return;

	const char *levelStr[] = { "DBG", "INF", "ERR", "FAT" };

	fprintf(log_file, "[%s]: ", levelStr[level]);

	va_list args;
	va_start(args, msg);
	vfprintf(log_file, msg, args);
	fprintf(log_file, "\n");
	va_end(args);
	
	if (level == LOG_FAT)
		abort();
}

void
entry_free(struct entry *entry)
{
	free(entry->hashes);
	entry->nhash = 0;
	free(entry->url);
}

void
user_free(struct user *user)
{
	for (int i=0; i < user->nentry; i++) {
		entry_free(&user->entries[i]);
	}
	
	user->nentry = 0;
	
	free(user->entries);
	free(user->email);
	free(user->smtp_domain);
	free(user->smtp_from);
	free(user->smtp_login);
	free(user->smtp_pwd);
	free(user->smtp_url);
}

static void
write_hashes_callback(void *user, struct post *post)
{
	log(LOG_DBG, "post %s with hash %lu\n", post->title, post->hash);
	fwrite(&post->hash, 1, sizeof(post->hash), (FILE *)user);
}

int
hash_cmp(const void *a, const void *b)
{
	return *(uint64_t *)a - *(uint64_t *)b;
}

size_t
load_hashes(uint64_t **hashes, struct user *user, const char *url)
{
	char buf[BUFSIZ] = {0};
	DIR *dir;

	if ((dir = opendir("/var/lib/mmre")) == NULL) {
		log(LOG_DBG, "Creating dir /var/lib/mmre.");
		mkdir("/var/lib/mmre", 0777);
	} else {
		closedir(dir);
	}
	
	snprintf(buf, BUFSIZ - 1, "/var/lib/mmre/%s", user->email);
	if ((dir = opendir(buf)) == NULL) {
		log(LOG_DBG, "Creating dir %s.", buf);
		mkdir(buf, 0777);
	} else {
		closedir(dir);
	}
	
	snprintf(buf, BUFSIZ - 1, "/var/lib/mmre/%s/%lx", user->email, djb2(url));
	FILE *f = fopen(buf, "rb");
	if (f != NULL) {
		log(LOG_DBG, "%s was found, reading from it.", buf);
		fseek(f, 0, SEEK_END);
		const size_t siz = ftell(f);
		fseek(f, 0, SEEK_SET);
		
		*hashes = malloc(siz);

		fread(*hashes, 1, siz, f);
		
		const size_t count = siz / sizeof(uint64_t);
		
		qsort(*hashes, count, sizeof(uint64_t), hash_cmp);
		
		return count;
	}
	
	f = fopen(buf, "wb");
	assertf(f, "Could not open file %s\n.", buf);
	log(LOG_DBG, "%s was not found, downloading feed from %s.", buf, url);
	char *data = get_url(url);
	assertg(fail, !parse_RSS(url, data, write_hashes_callback, f), "Could not parse RSS");
	
	fclose(f);
	free(data);
	
	return load_hashes(hashes, user, url);
fail:
	fclose(f);
	free(data);
	
	return 1;
}

void
save_hash(struct entry *entry, uint64_t hash)
{
	log(LOG_DBG, "Saving %lx as read.", hash);
	entry->hashes = realloc(entry->hashes, ++entry->nhash * sizeof(uint64_t));
	entry->hashes[entry->nhash - 1] = hash;
	
	char buf[BUFSIZ] = {0};
	snprintf(buf, BUFSIZ - 1, "/var/lib/mmre/%s/%lx", entry->owner->email, djb2(entry->url));
	FILE *f = fopen(buf, "wb");
	assertr(, f, "Could not open file %s.", buf);
	fwrite(entry->hashes, sizeof(uint64_t), entry->nhash, f);
	fclose(f);
}

int
asprintf(char **buf, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	
	char c;
	size_t siz = vsnprintf(&c, 1, fmt, args) + 1;
	*buf = malloc(siz);
	(*buf)[siz - 1] = 0;
	
	vsnprintf(*buf, siz, fmt, args);

	va_end(args);
	
	return siz;
}