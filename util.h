#ifndef _DS_STORE_H_
#define _DS_STORE_H_

#include <sys/types.h>
#include <minix/config.h>
#include <minix/ds.h>
#include <minix/bitmap.h>
#include <minix/param.h>
#include <regex.h>

#define NR_DS_KEYS	(2*NR_SYS_PROCS)
#define NR_DS_SUBS	(4*NR_SYS_PROCS)

struct data_store {
	int	flags;
	char	key[DS_MAX_KEYLEN];
	char	owner[DS_MAX_KEYLEN];

	union {
		unsigned u32;
		struct {
			void *data;
			size_t length;
			size_t reallen;
		} mem;
	} u;
};

struct subscription {
	int		flags;
	char		owner[DS_MAX_KEYLEN];
	regex_t		regex;
	bitchunk_t	old_subs[BITMAP_CHUNKS(NR_DS_KEYS)];	
};

int init_loggers();

#endif

#ifndef TRACE
#include "config.h"
#endif