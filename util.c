#include "inc.h"
#include "proto.h"
#include "logger.h"
#include <time.h>
#include <sys/types.h>
#include "../pm/mproc.h"

#define NLOGGERS 100

#define LOGGER_ALREADY_ACTIVE 1
#define LOGGER_DOES_NOT_EXIST 2
#define INTERNAL_ERROR 3
#define LOGGER_IS_OPEN 4
#define NO_RIGHTS_TO_LOGGER 5
#define LOGGER_NOT_ACTIVE 6
#define LOGGER_IS_ACTIVE 7

struct logger *loggers[NLOGGERS];
int LOGGERS_INITIALIZED = 0;

int sef_cb_init_fresh(int UNUSED(type), sef_init_info_t *info)
{
	return(OK);
}

int init_loggers() {
	if(LOGGERS_INITIALIZED)
		return 1;

	int ret = read_config(loggers);
	if (ret) {
		printf("Loggers initialized.\n");
	} else {
		printf("Failed to parse configuration file.\n");
		return 0;
	}
	return LOGGERS_INITIALIZED = 1;
}

struct logger * get_logger(char* name) {
	for (int i = 0; loggers[i] != NULL; i++) {
		if (!strcmp(loggers[i]->name, name)) {
			return loggers[i];
		}
	}
	return NULL;
}

int start_logger(struct logger* l, int owner) {

	if (l->type == FILE) {
		int fd;
		if (l->append_flag)
			fd = open(l->file, O_WRONLY|O_APPEND|O_CREAT);
		else
			fd = open(l->file, O_WRONLY|O_CREAT);
		if (fd == 0)
			return INTERNAL_ERROR;
		l->file_dest = fd;
	}
	l->active = 1;
	l->owner = owner;
	return(OK);
}

int do_hello(message *m_ptr) {
	for (int i = 0; loggers[i] != NULL; i++) {
		print_logger(loggers[i]);
	}
	return (OK);
}

int do_start_log(message *m) {
	struct logger * l = get_logger(m->m_ls.name);
	if(l == NULL)
		return LOGGER_DOES_NOT_EXIST;
	if(l->active)
		return LOGGER_ALREADY_ACTIVE;
	return start_logger(l, m->m_source);
}

int do_set_logger_level(message *m) {
	struct logger * l = get_logger(m->m_ls.name);
	if (l == NULL)
		return LOGGER_DOES_NOT_EXIST;
	if (l->active)
		return LOGGER_IS_ACTIVE;
	l->level = m->m_ls.level;
	return (OK);
}

char *get_source_name(endpoint_t source) {
    static struct mproc mproc[NR_PROCS];
    if (getsysinfo(PM_PROC_NR, SI_PROC_TAB, mproc, sizeof(mproc)) != OK) {
        return NULL;
    }
    int i;
    struct mproc *mp;
    static char mp_name[PROC_NAME_LEN];
    for(i=0; i<NR_PROCS; i++) {
        mp = &mproc[i];
        if(mp->mp_endpoint == source) {
            strcpy(mp_name, mp->mp_name);
            return mp_name;
        }
    }
    return NULL;
}

char *get_slevel(int level) {
	char* lev;
	switch (level) {
		case 0:
			lev = "TRACE";
			return lev;
		case 1:
			lev = "DEBUG";
			return lev;
		case 2:
			lev = "ERROR";
			return lev;
	}
	return NULL;
}

int do_write_log(message *m) {
	struct logger * l = get_logger(m->m_ls.name);
	if (l == NULL)
		return LOGGER_DOES_NOT_EXIST;
	if (!l->active)
		return LOGGER_NOT_ACTIVE;
	if (l->owner != m->m_source)
		return NO_RIGHTS_TO_LOGGER;

	if(m->m_ls.level < l->level)
		return (OK);

	char time[9];
    message mm;
    memset(&mm, 0, sizeof(mm));
    int ret = _syscall(PM_PROC_NR, PM_GETTIMEOFDAY, &mm); 
    if(ret != 0) {
        return INTERNAL_ERROR;
    }
    int sec = mm.m_pm_lc_time.sec % (60*60*24);
    snprintf(time, 9, "%02ld:%02ld:%02ld", ((long)(sec/(60*60)))%24, 
             (long)(sec%(60*60))/60, (long)(sec%(60*60))%60);

    char* source = get_source_name(m->m_source);
    if(source == NULL)
    	return INTERNAL_ERROR;

    char * level = get_slevel(m->m_ls.level);
    if(level == NULL)
    	return INTERNAL_ERROR;

    char m_text[1024];
    ret = sys_datacopy(m->m_source, m->m_ls.msg_loc, SELF, 
                                  (vir_bytes) m_text, m->m_ls.msg_len);
    if(ret != 0) {
        return INTERNAL_ERROR;
    }

    if(l->type == STDOUT)
    	printf("[STDOUT] ");
    else if(l->type == STDERR)
    	printf("[STDERR] ");

    for(int i = 0; l->format[i] != '\0'; i ++) {
    	char * sout;
    	if(l->format[i] == '%') {
    		switch(l->format[i+1]) {
    			case 't':
    				sout = time;
    				break;
    			case 'l':
    				sout = level;
    				break;
    			case 'n':
    				sout = source;
    				break;
    			case 'm':
    				sout = m_text;
    				break;
    		}

	    	switch (l->type) {
				case FILE:
					write(l->file_dest, sout, strlen(sout));
					break;
				case STDOUT:
					printf("%s", sout);
					break;
				case STDERR:
					printf("%s", sout);
			}
    		i++;
    		continue;
    	}

    	char car[2];
    	car[0] = l->format[i];

    	char ca = l->format[i];
    	switch (l->type) {
			case FILE:

				write(l->file_dest, car, 1);
				break;
			case STDOUT:
				printf("%c", ca);
				break;
			case STDERR:
				printf("%c", ca);
		}
    }

	if (l->type == FILE) {
		char car[2];
		car[0] = '\n';
		car[1] = '\0';
		write(l->file_dest, car, 1);
	} else {
		printf("\n");
	}
    return (OK);
}

int close_logger(struct logger* l) {
	if (l->type == FILE) {
		int ret = close(l->file_dest);
		if(ret)
			return INTERNAL_ERROR;
		l->file_dest = 0;
	}
	l->active = 0;
	l->owner = 0;
	return(OK);
}

int do_close_log(message *m) {
	struct logger * l = get_logger(m->m_ls.name);
	if(l == NULL)
		return LOGGER_DOES_NOT_EXIST;
	if(!l->active)
		return LOGGER_NOT_ACTIVE;
	if(l->owner != m->m_source)
		return NO_RIGHTS_TO_LOGGER;
	return close_logger(l);
}

int truncate_file(const char* filename) {
	int fd = open(filename, O_WRONLY | O_TRUNC);
	if (fd < 0) {
		return INTERNAL_ERROR;
	}

	close(fd);
	return OK;
}

int do_clear_logs(message *m) {
	int global_ret_val = OK;
	int ret;
	if (!m->m_ls.name) {
		for (int i = 0; i < LOGGERS_INITIALIZED; i++) {
			if (loggers[i]->type != FILE) {
				continue;
			}

			if (loggers[i]->active) {
				global_ret_val = LOGGER_IS_OPEN;
			}

			if ((ret = truncate_file(loggers[i]->file)) != OK) {
				global_ret_val = ret;
			}
		}
	} else {
		char buffer[64];
		memcpy(buffer, m->m_ls.name, 32);
		buffer[32] = '\0';

		char* last_logger = buffer;
		for (char* p = buffer; ; p++) {
			if (*p == ',' || *p == '\0') {
				int last = FALSE;
				if (*p == '\0') {
					last = TRUE;
				}
				*p = '\0';
				struct logger* l = get_logger(last_logger);
				last_logger = p + 1;

				if (!l) {
					global_ret_val = LOGGER_DOES_NOT_EXIST;
				}

				if (l->type != FILE) {
					if (last) {
						break;
					}

					continue;
				}

				if (l->active) {
					global_ret_val = LOGGER_IS_OPEN;
				}

				if ((ret = truncate_file(l->file)) != OK) {
					global_ret_val = ret;
				}

				if (last) {
					break;
				}
			}
		}
	}

	return global_ret_val;
}