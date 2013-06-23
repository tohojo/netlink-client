/**
 * options.h
 *
 * Toke Høiland-Jørgensen
 * 2013-06-04
 */

#ifndef OPTIONS_H
#define OPTIONS_H

#include <sys/time.h>
#include <unistd.h>
#include <netlink/route/link.h>
#include <errno.h>

#include "formatter.h"


struct options {
	int if_c;
	int *ifs;
	int run_length;
	struct timeval start_time;
	struct nl_sock *sk_req;
	struct nl_sock *sk_listen;
	struct nl_cache *cache;
	struct formatter *formatter;
};

#include "netlink_comm.h"


int initialise_options(struct options *opt, int argc, char **argv);

#endif
