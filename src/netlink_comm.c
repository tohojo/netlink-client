/**
 * netlink_comm.c
 *
 * Toke Høiland-Jørgensen
 * 2013-06-04
 */

#include "netlink_comm.h"

static int netlink_msg_handler(struct nl_msg *msg, void *arg);



struct nl_sock *create_socket()
{
	return nl_socket_alloc();
}

void destroy_socket(struct nl_sock *sk)
{
	nl_socket_free(sk);
}

int setup_socket(struct nl_sock *sk)
{
	int ret;
	nl_socket_disable_seq_check(sk);

	if((ret = nl_socket_modify_cb(sk, NL_CB_VALID, NL_CB_CUSTOM, netlink_msg_handler, NULL)) < 0)
		return ret;

	if((ret = nl_connect(sk, NETLINK_ROUTE)) < 0)
		return ret;

	if((ret = nl_socket_add_memberships(sk, RTNLGRP_TC_STATS, 0)) < 0)
		return ret;
	return 0;
}


static int netlink_msg_handler(struct nl_msg *msg, void *arg)
{
  printf("Handler called...\n");
  return 0;
}
