#ifndef _AF_DEBUG_
#define _AF_DEBUG_

#include <stdio.h>
#include <stdlib.h>

#ifdef DEBUG
#define dbg_printf(fmt,...) \
	printf("[debug: %s] " fmt, __FUNCTION__, ##__VA_ARGS__)
#else
#define dbg_printf(fmt,...)
#endif

#define err_printf(fmt,...) \
	fprintf(stderr, "[error: %s] " fmt, __FUNCTION__, ##__VA_ARGS__)

#define abort_printf(fmt,...) \
	do { printf("[aborted: %s] " fmt, __FUNCTION__, ##__VA_ARGS__); exit(0); } \
	while (0)

#endif
