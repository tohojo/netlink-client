/**
 * main.c
 *
 * Toke Høiland-Jørgensen
 * 2013-06-03
 */

#include <stdio.h>
#include <netlink/netlink.h>
#include <netlink/socket.h>

static int handler(struct nl_msg *msg, void *arg)
{
  printf("Handler called...\n");
  return 0;
}


int main()
{
  printf("Initialising...\n");

  struct nl_sock *sk;

  sk = nl_socket_alloc();
  nl_socket_disable_seq_check(sk);
  nl_socket_modify_cb(sk, NL_CB_VALID, NL_CB_CUSTOM, handler, NULL);

  nl_connect(sk, NETLINK_ROUTE);

  if(nl_socket_add_memberships(sk, RTNLGRP_TC_STATS, 0))
    goto out;

  printf("Joined mcast group...\n");

  while(1)
    nl_recvmsgs_default(sk);

out:
  nl_socket_free(sk);
}
