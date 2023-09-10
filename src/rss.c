#include <stdio.h>
#include <string.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <mmre.h>

uint64_t
hashPost(struct post *post) {
	return djb2(post->title ? post->title : post->link);
}

static char *nullStr = "(null)";

static void
parseRssPost(struct post *post, xmlNode *root) {
	for (xmlNode *node = root->children; node; node = node->next) {
		if (strcmp((char *)node->name, "title") == 0) {
			if (node->children == NULL)
				post->title = nullStr;
			else
				post->title = (char *)node->children->content;
		} else if (strcmp((char *)node->name, "link") == 0) {
			assertr(, node->children, "The link node is invalid.");
			post->link = (char *)node->children->content;
		}
	}
}

// same as rss but the link is in 'href' attribute instead of link node content
static void
parseAtomPost(struct post *post, xmlNode *root) {
	for (xmlNode *node = root->children; node; node = node->next) {
		if (strcmp((char *)node->name, "title") == 0) {
			if (node->children == NULL)
				post->title = nullStr;
			else
				post->title = (char *)node->children->content;
		} else if (strcmp((char *)node->name, "link") == 0) {
			for (xmlAttr *attr = node->properties; attr;
			     attr = attr->next) {
				if (strcmp((char *)attr->name, "href") == 0) {
					post->link =
					    (char *)attr->children->content;
					goto ok;
				}
			}
			assertr(
			    , node->children,
			    "The link node has no href attribute."
			);
		ok:;
		}
	}
}

int
parse_RSS(
    const char *url, const char *text,
    void (*callback)(void *user, struct post *), void *user
) {
	xmlDoc *doc;
	xmlNode *root;

	doc = xmlReadDoc((xmlChar *)text, url, NULL, 0);
	assertg(fail, doc, "Could not parse %s.", url);

	struct feed feed = {0};

	root = xmlDocGetRootElement(doc);
	xmlNode *channel = root; // atom has item nodes in the root

	// if this is not atom feed, search for the channel node
	for (xmlNode *node = root->children; node; node = node->next) {
		if (strcmp((char *)node->name, "channel") == 0) {
			channel = node;
			break;
		}
	}

	assertg(fail, channel, "The RSS feed %s is invalid.", url);

	for (xmlNode *node = channel->children; node; node = node->next) {
		if (strcmp((char *)node->name, "title") == 0) {
			feed.name = (char *)node->children->content;
		} else if (strcmp((char *)node->name, "item") == 0) { // RSS
			struct post post = {.parent = &feed};
			parseRssPost(&post, node);
			post.hash = hashPost(&post);
			callback(user, &post);
		} else if (strcmp((char *)node->name, "entry") == 0) { // atom
			struct post post = {.parent = &feed};
			parseAtomPost(&post, node);
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
check_entry_rss_callback(void *user, struct post *post) {
	struct entry *entry = user;
	const struct user *owner = entry->owner;

	if (bsearch(
		&post->hash, entry->hashes, entry->nhash, sizeof(uint64_t),
		hash_cmp
	    ))
		return;

	log(LOG_INF, "New unread post %s %lx from feed %s.", post->title,
	    post->hash, entry->url);

	char *subject;
	asprintf(&subject, "%s: %s", post->parent->name, post->title);

	int ret = send_email_SMTP(
	    smtp_url, subject, smtp_domain, smtp_from, owner->email, smtp_login,
	    smtp_pwd, post->link
	);

	free(subject);

	if (!ret) {
		save_hash(entry, post->hash);
	}
}

void
check_entry(struct user *user, struct entry *entry) {
	char *data = get_url(entry->url);
	if (data == NULL)
		return;
	assertg(
	    cleanup,
	    !parse_RSS(entry->url, data, check_entry_rss_callback, entry),
	    "Could not parse feed from %s.", entry->url
	);

cleanup:
	free(data);
}
