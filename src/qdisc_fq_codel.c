/**
 * qdisc_fq_codel.c
 *
 * Toke Høiland-Jørgensen
 * 2013-06-22
 */

#include <linux/pkt_sched.h>

#include "netlink_comm.h"

static int fq_codel_print_stats(struct nlattr *attr)
{
	struct tc_fq_codel_xstats *st;
	struct tc_fq_codel_flow_stats *fst;
	int i;

	if(nla_len(attr) < sizeof(*st))
		return -1;

	st = nla_data(attr);
	fst = (void*)st + sizeof(*st);
	if(st->type != TCA_FQ_CODEL_XSTATS_QDISC) {
		printf("Warning: Invalid type for fq_codel stats!\n");
		return -1;
	}

	printf("  maxpacket: %u drop_overlimit: %u ecn_mark: %u new_flow_count %u\n",
		st->qdisc_stats.maxpacket,
		st->qdisc_stats.drop_overlimit,
		st->qdisc_stats.ecn_mark,
		st->qdisc_stats.new_flow_count);
	printf("  new_flows_len: %u old_flows_len: %u\n",
		st->qdisc_stats.new_flows_len,
		st->qdisc_stats.old_flows_len);
	for(i = 0; i < st->qdisc_stats.act_flows_count; i++) {
		printf("  flow %u: qlen %up %ub delay %u",
			fst->flow_id,
			fst->qlen,
			fst->backlog,
			fst->delay);
		fst++;
	}
	return 0;
}

struct qdisc_handler fq_codel_qdisc_handler = {
	.id = "fq_codel",
	.print_stats = fq_codel_print_stats,
};
