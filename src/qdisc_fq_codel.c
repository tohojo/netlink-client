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
	struct recordset nested_rset = {0};
	int i;


	if(nla_len(attr) < sizeof(*st))
		return -1;

	st = nla_data(attr);
	fst = (void*)st + sizeof(*st);
	if(st->type != TCA_FQ_CODEL_XSTATS_QDISC) {
		printf("Warning: Invalid type for fq_codel stats!\n");
		return -1;
	}


	add_record_uint(rset, "maxpacket", sizeof("maxpacket"), st->qdisc_stats.maxpacket);
	add_record_uint(rset, "drop_overlimit", sizeof("drop_overlimit"), st->qdisc_stats.drop_overlimit);
	add_record_uint(rset, "ecn_mark", sizeof("ecn_mark"), st->qdisc_stats.ecn_mark);
	add_record_uint(rset, "new_flow_count", sizeof("new_flow_count"), st->qdisc_stats.new_flow_count);
	add_record_uint(rset, "new_flows_len", sizeof("new_flows_len"), st->qdisc_stats.new_flows_len);
	add_record_uint(rset, "old_flows_len", sizeof("old_flows_len"), st->qdisc_stats.old_flows_len);

	if(st->qdisc_stats.act_flows_count) {

		for(i = 0; i < st->qdisc_stats.act_flows_count; i++, fst++) {
			nested_rset.len = 0;
			nested_rset.records = NULL;
			add_record_uint(&nested_rset, "id", sizeof("id"), fst->flow_id);
			add_record_uint(&nested_rset, "qlen", sizeof("qlen"), fst->qlen);
			add_record_uint(&nested_rset, "backlog", sizeof("backlog"), fst->backlog);
			add_record_uint(&nested_rset, "delay", sizeof("delay"), fst->delay);
			add_record_rset(rset, "flows", sizeof("flows"), &nested_rset);
		}
	}
	return 0;
}

struct qdisc_handler fq_codel_qdisc_handler = {
	.id = "fq_codel",
	.parse_stats = fq_codel_parse_stats,
};
