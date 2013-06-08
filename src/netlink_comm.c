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

int setup_socket(struct nl_sock *sk, struct options *opt)
{
	int ret;
	nl_socket_disable_seq_check(sk);

	if((ret = nl_socket_modify_cb(sk, NL_CB_VALID, NL_CB_CUSTOM, netlink_msg_handler, opt)) < 0)
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
	struct timeval current_time;

	struct gnet_stats_basic *sb;
	struct gnet_stats_queue *q;

	struct options *opt = arg;

	hdr = nlmsg_hdr(msg);
	tcm = nlmsg_data(hdr);
	if(!has_iface(opt, tcm->tcm_ifindex))
		return 0;

	gettimeofday(&current_time, NULL);
	printf("[%lu.%06lu] ",
		(unsigned long)current_time.tv_sec,
		(unsigned long)current_time.tv_usec);


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
	if(current_time.tv_sec < opt->start_time.tv_sec + opt->run_length)
		return NL_OK;
	else
		return NL_STOP;
}
