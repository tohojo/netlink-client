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
	struct nlmsghdr *hdr;
	struct tcmsg *tcm;
	struct nlattr *attrs[TCA_MAX+1];
	struct nlattr *stat_attrs[TCA_STATS_MAX+1];
	char qdisc[IFNAMSIZ];

	struct gnet_stats_basic *sb;
	struct gnet_stats_queue *q;

	printf("Handler called...\n");

	hdr = nlmsg_hdr(msg);
	tcm = nlmsg_data(hdr);
	printf("TCM: family: %d, ifindex: %d, parent: %d, handle: %d\n",
		tcm->tcm_family,
		tcm->tcm_ifindex,
		tcm->tcm_parent,
		tcm->tcm_handle);
	nlmsg_parse(hdr, sizeof(*tcm), attrs, TCA_MAX, tca_policy);

	if(attrs[TCA_KIND]) {
		strcpy(qdisc, nla_get_string(attrs[TCA_KIND]));
		printf("Assigned qdisc: %s\n", qdisc);
	}

	if(attrs[TCA_STATS2]) {
		nla_parse_nested(stat_attrs, TCA_STATS_MAX, attrs[TCA_STATS2], tca_stats_policy);
		if(stat_attrs[TCA_STATS_BASIC]) {
			sb = nla_data(stat_attrs[TCA_STATS_BASIC]);
			printf("Bytes: %d, packets: %d\n", sb->bytes, sb->packets);
		}

		if(stat_attrs[TCA_STATS_QUEUE]) {
			q = nla_data(stat_attrs[TCA_STATS_QUEUE]);
			printf("Drops: %d, qlen: %d, backlog: %d, overlimits: %d, requeues: %d\n",
				q->drops,
				q->qlen,
				q->backlog,
				q->overlimits,
				q->requeues);
		}
	}
	return 0;
}
