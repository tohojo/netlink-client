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

#include "netlink_comm.h"

struct options {
	int if_c;
	int *ifs;
	struct timeval start_time;
	struct nl_sock *sk_req;
	struct nl_sock *sk_listen;
	struct nl_cache *cache;
};

int initialise_options(struct options *opt, int argc, char **argv);

#endif
