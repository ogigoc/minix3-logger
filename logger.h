#include <lib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "inc.h"
#include <fcntl.h>
#include <lib.h>
#include <time.h>
#include <sys/types.h>

#define FILE 0
#define STDOUT 1
#define STDERR 2

#define TRACE 0
#define DEBUG 1
#define ERROR 2

struct logger {
	char *name;
	int type;

	char *file;
	int append_flag;
	int file_dest;

	int level;
	char *format;

	int active;
	endpoint_t owner;
};

int read_config();
void print_logger(struct logger* l);