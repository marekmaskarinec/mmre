#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <curl/curl.h>

#define MMRE_NO_GLOB
#include <mmre.h>

FILE *log_file = NULL;
int min_log_level;
uint32_t interval;
char *smtp_login = NULL;
char *smtp_pwd = NULL;
char *smtp_from = NULL;
char *smtp_url = NULL;
char *smtp_domain = NULL;
struct user *users;
int nuser;

static char *cfg_path = "/etc/mmre.ini";

static void
help() {
	printf(
	    "mmre " VERSION " - a rss to email daemon\n"
	    "\t-c <path> - set the config path. Default: /etc/mmre.ini\n"
	    "\t-l <path> - set the log path. Default: stdout\n"
	    "\t-v - enable verbose output\n"
	    "\t-h - show this message\n"
	    "see mmre(1) and mmre(5) for more info\n"
	    "https://mmq.cz/mmre\n"
	);
	exit(0);
}

extern char *optarg;
extern int optind, opterr, optopt;

void
SIGINT_handler(int _) {
	exit(0);
}

static void
destroy(void) {
	log(LOG_INF, "Exiting");
	fflush(log_file);
	if (log_file != stderr)
		fclose(log_file);

	for (int i = 0; i < nuser; ++i) {
		user_free(&users[i]);
	}

	free(users);
}

int
main(int argc, char *argv[]) {
	char *log_path = "-";
	min_log_level = 1;

	int opt;
	while ((opt = getopt(argc, argv, "l:c:vh")) != -1) {
		switch (opt) {
		case 'l':
			log_path = optarg;
			break;
		case 'c':
			cfg_path = optarg;
			break;
		case 'v':
			min_log_level = 0;
			break;
		case 'h':
		case '?':
			help();
			break;
		}
	}

	if (strcmp("-", log_path) != 0) {
		log_file = fopen(log_path, "a");
		if (log_file == NULL) {
			log(LOG_INF, "Could not open %s. Using stdout instead.",
			    log_path);
		}
	}

	if (log_file == NULL) {
		log_file = stdout;
	}

	log(LOG_INF, "Starting mmre " VERSION);

	load_config(cfg_path);

	atexit(destroy);
	signal(SIGINT, SIGINT_handler);
	signal(SIGTERM, SIGINT_handler);

	time_t mtime = 0;

	for (;;) {
		struct stat st;
		if (stat(cfg_path, &st) == 0) {
			if (mtime == 0)
				mtime = st.st_mtime;

			if (mtime != st.st_mtime) {
				mtime = st.st_mtime;
				load_config(cfg_path);
			}
		}

		for (int i = 0; i < nuser; ++i)
			for (int j = 0; j < users[i].nentry; ++j)
				check_entry(&users[i], &users[i].entries[j]);

		sleep(interval * 60);
	}
}
