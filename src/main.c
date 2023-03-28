#include <stdlib.h>

#include <curl/curl.h>

#define MMRE_NO_GLOB
#include <mmre.h>

FILE *log_file;
int min_log_level;
uint32_t interval;
struct user *users;
int nuser;

int main(int argc, char *argv[])
{
	log_file = stderr;
	
	load_config("/etc/mmre.ini");
	
	check_entry(&users[0], &users[0].entries[0]);
	check_entry(&users[0], &users[0].entries[1]);
}
