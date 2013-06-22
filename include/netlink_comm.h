/**
 * netlink_comm.h
 *
 * Toke Høiland-Jørgensen
 * 2013-06-04
 */

#ifndef NETLINK_COMM_H
#define NETLINK_COMM_H

#include <netlink/netlink.h>
#include <netlink/socket.h>

#include "options.h"

struct nl_sock *create_socket();
void destroy_socket(struct nl_sock *sk);
int setup_socket(struct nl_sock *sk, struct options *opt);

struct qdisc_handler {
	struct  qdisc_handler *next;
	const char *id;
	int 	(*print_stats)(struct nlattr *stats);
};

struct qdisc_handler *find_qdisc_handler(const char *name);

#endif
