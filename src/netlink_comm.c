/**
 * netlink_comm.c
 *
 * Toke Høiland-Jørgensen
 * 2013-06-04
 */

#include <dlfcn.h>
#include <stdio.h>

#include <netlink/msg.h>
#include <netlink/netlink.h>
#include <netlink/attr.h>
#include <linux/gen_stats.h>

#include "netlink_comm.h"
#include "formatter.h"

static struct nla_policy tca_policy[TCA_MAX+1] = {
        [TCA_KIND] = { .type = NLA_STRING,
                          .maxlen = IFNAMSIZ },
        [TCA_STATS2] = { .type = NLA_NESTED },
};

static struct nla_policy tca_stats_policy[TCA_STATS_MAX+1] = {
  [TCA_STATS_BASIC] = { .minlen = sizeof(struct gnet_stats_basic)},
  [TCA_STATS_QUEUE] = { .minlen = sizeof(struct gnet_stats_queue)},
};

static struct qdisc_handler *qdisc_handler_list;

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
	struct qdisc_handler *h;
	char qdisc[IFNAMSIZ] = {0};
	char ifname[IFNAMSIZ] = {0};
	struct timeval current_time = {0};

	char *ret = NULL;
	char buf[128] = {0};

	struct gnet_stats_basic *sb;
	struct gnet_stats_queue *q;

	struct options *opt = arg;

	struct recordset rset = {0};

	hdr = nlmsg_hdr(msg);
	tcm = nlmsg_data(hdr);
	if(!has_iface(opt, tcm->tcm_ifindex))
		return NL_SKIP;

	if((ret = rtnl_link_i2name(opt->cache, tcm->tcm_ifindex, ifname, IFNAMSIZ)) == NULL)
		return NL_SKIP;

	gettimeofday(&current_time, NULL);
	snprintf(buf, sizeof(buf), "%lu.%06lu",
		(unsigned long)current_time.tv_sec,
		(unsigned long)current_time.tv_usec);
	add_record(&rset, "time", buf);
	add_record(&rset, "iface", ifname);

	nlmsg_parse(hdr, sizeof(*tcm), attrs, TCA_MAX, tca_policy);

	if(attrs[TCA_KIND]) {
		strcpy(qdisc, nla_get_string(attrs[TCA_KIND]));
		add_record(&rset, "qdisc", qdisc);
	}

	snprintf(buf, sizeof(buf), "%x",
		tcm->tcm_handle >> 16);
	add_record(&rset, "handle", buf);

	if(attrs[TCA_STATS2]) {
		nla_parse_nested(stat_attrs, TCA_STATS_MAX, attrs[TCA_STATS2], tca_stats_policy);
		if(stat_attrs[TCA_STATS_BASIC]) {
			sb = nla_data(stat_attrs[TCA_STATS_BASIC]);
			add_record_u(&rset, "bytes", sb->bytes);
			add_record_u(&rset, "packets", sb->packets);
		}

		if(stat_attrs[TCA_STATS_QUEUE]) {
			q = nla_data(stat_attrs[TCA_STATS_QUEUE]);
			add_record_u(&rset, "drops", 		q->drops);
			add_record_u(&rset, "qlen", 		q->qlen);
			add_record_u(&rset, "backlog", 	q->backlog);
			add_record_u(&rset, "overlimits", 	q->overlimits);
			add_record_u(&rset, "requeues", 	q->requeues);
		}
		if(stat_attrs[TCA_STATS_APP]) {
			h = find_qdisc_handler(qdisc);
			if(h)
				h->parse_stats(stat_attrs[TCA_STATS_APP], &rset);
		}
	}
	opt->formatter->format(opt->formatter, &rset);
	clear_records(&rset);
	if(current_time.tv_sec < opt->start_time.tv_sec + opt->run_length)
		return NL_OK;
	else
		return NL_STOP;
}

struct qdisc_handler *find_qdisc_handler(const char *name)
{
	char buf[128] = {0};
	void *dlh;
	struct qdisc_handler *h;

	for (h = qdisc_handler_list; h; h = h->next)
		if(strcmp(h->id, name) == 0)
			return h;

	dlh = dlopen(NULL, RTLD_LAZY);
	snprintf(buf, sizeof(buf), "%s_qdisc_handler", name);
	h = dlsym(dlh, buf);
	if(h) {
		h->next = qdisc_handler_list;
		qdisc_handler_list = h;
	}
	return h;
}
