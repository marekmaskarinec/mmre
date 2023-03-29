#include <string.h>
#include <stdio.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <mmre.h>

uint64_t
hashPost(struct post *post) {
	return djb2(post->title);
}

static void
parsePost(struct post *post, xmlNode *root) {
	for (xmlNode *node = root->children; node; node = node->next) {
		if (strcmp((char *)node->name, "title") == 0) {
			post->title = (char *)node->children->content;
		} else if (strcmp((char *)node->name, "link") == 0) {
			post->link = (char *)node->children->content;
		} else if (strcmp((char *)node->name, "pubDate") == 0) {
			post->date = (char *)node->children->content;
		} else if (strcmp((char *)node->name, "description") == 0) {
			post->content = (char *)node->children->content;
		}
	}
}

int
parse_RSS(const char *url, const char *text, void (* callback)(void *user, struct post *), void *user)
{
	xmlDoc *doc;
	xmlNode *root;

	doc = xmlReadDoc((xmlChar *)text, url, NULL, 0);
	assertg(fail, doc, "Could not parse %s.", url);
	
	struct feed feed = {0};
	
	root = xmlDocGetRootElement(doc);
	xmlNode *channel = {0};
	
	for (xmlNode *node = root->children; node; node = node->next)
		if (strcmp((char *)node->name, "channel") == 0)
			channel = node;
	
	assertg(fail, channel, "The RSS feed %s is invalid.", url);
	
	for (xmlNode *node = channel->children; node; node = node->next) {
		if (strcmp((char *)node->name, "title") == 0) {
			feed.name = (char *)node->children->content;
		} else if (strcmp((char *)node->name, "item") == 0) {
			struct post post = {
				.parent = &feed
			};
			parsePost(&post, node);
			post.hash = hashPost(&post);
			callback(user, &post);
		}
	}
	
cleanup:
	xmlFreeDoc(doc);
	xmlCleanupParser();
	
	return 0;
	
fail:
	xmlFreeDoc(doc);
	xmlCleanupParser();
	
	return 1;
}

static void
check_entry_rss_callback(void *user, struct post *post)
{
	struct entry *entry = user;
	const struct user *owner = entry->owner;
	
	if (bsearch(&post->hash, entry->hashes, entry->nhash, sizeof(uint64_t), hash_cmp))
		return;
	
	log(LOG_INF, "New unread post %s %lx from feed %s.", post->title, post->hash, entry->url);
	
	char *subject;
	asprintf(&subject, "%s: %s", post->parent->name, post->title);

	send_email_SMTP(
		owner->smtp_url, subject, owner->smtp_domain, owner->smtp_from,
		owner->email, owner->smtp_login, owner->smtp_pwd, post->link
	);
	
	free(subject);

	save_hash(entry, post->hash);
}

void
check_entry(struct user *user, struct entry *entry)
{
	char *data = get_url(entry->url);
	if (data == NULL)
		return;
	assertg(cleanup, !parse_RSS(entry->url, data, check_entry_rss_callback, entry),
		"Could not parse feed from %s.", entry->url);
		
cleanup:
	free(data);
}