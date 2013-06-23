/**
 * qdisc_fq_codel.c
 *
 * Toke Høiland-Jørgensen
 * 2013-06-22
 */

#include <linux/pkt_sched.h>

#include "formatter.h"
#include "netlink_comm.h"

static int fq_codel_parse_stats(struct nlattr *attr, struct recordset *rset)
{
	struct tc_fq_codel_xstats *st;
	struct tc_fq_codel_flow_stats *fst;
	int i;
	char buf[2][128] = {{0}, {0}};


	if(nla_len(attr) < sizeof(*st))
		return -1;

	st = nla_data(attr);
	fst = (void*)st + sizeof(*st);
	if(st->type != TCA_FQ_CODEL_XSTATS_QDISC) {
		printf("Warning: Invalid type for fq_codel stats!\n");
		return -1;
	}


	add_record_u(rset, "maxpacket", st->qdisc_stats.maxpacket);
	add_record_u(rset, "drop_overlimit", st->qdisc_stats.drop_overlimit);
	add_record_u(rset, "ecn_mark", st->qdisc_stats.ecn_mark);
	add_record_u(rset, "new_flow_count", st->qdisc_stats.new_flow_count);
	add_record_u(rset, "new_flows_len", st->qdisc_stats.new_flows_len);
	add_record_u(rset, "old_flows_len", st->qdisc_stats.old_flows_len);

	for(i = 0; i < st->qdisc_stats.act_flows_count; i++, fst++) {
		snprintf(buf[0], sizeof(buf[0]), "flow %u", fst->flow_id);
		snprintf(buf[1], sizeof(buf[1]), "qlen %up %ub delay %u",
			fst->qlen,
			fst->backlog,
			fst->delay);
		add_crecord(rset, buf[0], buf[1]);
	}
	return 0;
}

struct qdisc_handler fq_codel_qdisc_handler = {
	.id = "fq_codel",
	.parse_stats = fq_codel_parse_stats,
};
