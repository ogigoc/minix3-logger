#include "logger.h"

char * SFILE = "FILE";
char * SSTDOUT = "STDOUT";
char * SSTDERR = "STDERR";

char * STRACE = "TRACE";
char * SDEBUG = "DEBUG";
char * SERROR = "ERROR";

char * STRUE = "TRUE";
char * SFALSE = "FALSE";

void parse_row(char **dest, char **buff) {
    char *row_pos = strchr(*buff, '\n');
    if (row_pos == NULL)
            return;
	int len = row_pos - *buff;
	*dest = malloc(sizeof(char) * len);
	strncpy(*dest, *buff, len);
	*buff += len + 1;
}

int logger_from_string(struct logger *l, char **buff) {
	if (*buff[0] == '\0')
		return 0;
	char *line;
	parse_row(&(l->name), buff);
	
	parse_row(&line, buff);
	if (!strcmp(line, SFILE)) l->type = FILE;
	else if (!strcmp(line, SSTDOUT)) l->type = STDOUT;
	else if (!strcmp(line, SSTDERR)) l->type = STDERR;
	else return -1;

	if (l->type == FILE) {
		parse_row(&(l->file), buff);
		parse_row(&line, buff);
		if (!strcmp(line, STRUE)) l->append_flag = 1;
		else if (!strcmp(line, SFALSE)) l->append_flag = 0;
		else return -1;
	}
	
	parse_row(&line, buff);
	if (!strcmp(line, STRACE)) l->level = TRACE;
	else if (!strcmp(line, SDEBUG)) l->level = DEBUG;
	else if (!strcmp(line, SERROR)) l->level = ERROR;
	else return -1;

	parse_row(&(l->format), buff);
	
	return 1;
}

void print_logger(struct logger *l) {
	printf("%s\n", l->name);
	printf("%d\n", l->type);
	if(l->type == FILE) {
		printf("%s\n", l->file);
		printf("%d\n", l->append_flag);
	}
	printf("%d\n", l->level);
	printf("%s\n", l->format);
}

int read_config(struct logger **loggers) {
	char *filename = "/etc/ls.config";
	int fd = open(filename, O_RDONLY);
 	
	ssize_t size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	char *buff = malloc(size + 1);

	int nread = read(fd, buff, size); 
    
    int i, flag;
	for(i = 0, flag = 1; (loggers[i] = malloc(sizeof(struct logger))) && \
	 (flag = logger_from_string(loggers[i], &buff)); i ++) {
	 	if (flag == -1) {
	 		printf("Parsing of config file failed.\n");
	 		return 0;
	 	}
	}
	free(loggers[i]);
	loggers[i] = NULL;

	return 1;
}