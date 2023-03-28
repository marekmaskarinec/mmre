#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <curl/curl.h>

#define MMRE_NO_GLOB
#include <mmre.h>

FILE *log_file;
int min_log_level;
uint32_t interval;
struct user *users;
int nuser;

static char *cfg_path = "/etc/mmre.ini";

static volatile int alive = 1;

static void
SIGINT_handler(int _)
{
	alive = 0;
}

static void
SIGHUP_handler(int _)
{
	log(LOG_INF, "Reloading config");
	load_config(cfg_path);
}

static void
help()
{
	printf("mmre - a rss to email daemon\n"
		"\t-c <path> - set the config path. Default /etc/mmre.ini\n"
		"\t-l <path> - set the log path. Default /var/log/mmre.log\n"
		"\t-v - enable verbose output\n"
		"see mmre(1) and mmre(5) for more info\n"
		"https://mrms.cz/mmre\n");
	exit(0);
}

extern char *optarg;
extern int optind, opterr, optopt;

int main(int argc, char *argv[])
{
	char *log_path = "/var/log/mmre.log";
	min_log_level = 1;
	
	int opt;
	while ((opt = getopt(argc, argv, "l:c:v")) != -1) {
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
		case '?':
			help();
			break;
		}
	}

	log_file = fopen(log_path, "a");
	if (log_file == NULL) {
		log_file = stderr;
		log(LOG_INF, "Could not open %s. Using stderr instead.", log_path);
	}
	
	load_config(cfg_path);
	
	signal(SIGINT, SIGINT_handler);
	signal(SIGHUP, SIGHUP_handler);
	signal(SIGTERM, SIGINT_handler);
	
	while (alive) {
		for (int i=0; i < nuser; ++i)
			for (int j=0; j < users[i].nentry; ++j)
				check_entry(&users[i], &users[i].entries[j]);
		
		sleep(interval * 60);
	}

	log(LOG_INF, "exiting");

	if (log_file != stderr)
		fclose(log_file);	
	
	for (int i=0; i < nuser; ++i) {
		user_free(&users[i]);
	}
	
	free(users);
}
