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
	char ifname[IFNAMSIZ];
	struct timeval current_time;

	char *ret = NULL;

	struct gnet_stats_basic *sb;
	struct gnet_stats_queue *q;

	struct options *opt = arg;

	hdr = nlmsg_hdr(msg);
	tcm = nlmsg_data(hdr);
	if(!has_iface(opt, tcm->tcm_ifindex))
		return NL_OK;

	if((ret = rtnl_link_i2name(opt->cache, tcm->tcm_ifindex, ifname, IFNAMSIZ)) == NULL)
		return NL_SKIP;

	gettimeofday(&current_time, NULL);
	printf("[%lu.%06lu] ",
		(unsigned long)current_time.tv_sec,
		(unsigned long)current_time.tv_usec);

	printf("dev: %s ", ifname);


	nlmsg_parse(hdr, sizeof(*tcm), attrs, TCA_MAX, tca_policy);

	if(attrs[TCA_KIND]) {
		strncpy(qdisc, nla_get_string(attrs[TCA_KIND]), IFNAMSIZ-1);
		printf("qdisc: %s ", qdisc);
	}

	printf("parent: %d handle: %d ",
		tcm->tcm_parent,
		tcm->tcm_handle);

	if(attrs[TCA_STATS2]) {
		nla_parse_nested(stat_attrs, TCA_STATS_MAX, attrs[TCA_STATS2], tca_stats_policy);
		if(stat_attrs[TCA_STATS_BASIC]) {
			sb = nla_data(stat_attrs[TCA_STATS_BASIC]);
			printf("transfer: %ub %up\n", sb->bytes, sb->packets);
		}

		if(stat_attrs[TCA_STATS_QUEUE]) {
			q = nla_data(stat_attrs[TCA_STATS_QUEUE]);
			printf("Drops: %u qlen: %up %ub overlimits: %u requeues: %u\n",
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
