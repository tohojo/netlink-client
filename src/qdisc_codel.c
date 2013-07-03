/**
 * qdisc_codel.c
 *
 * Toke Høiland-Jørgensen
 * 2013-07-03
 */

#include <linux/pkt_sched.h>

#include "formatter.h"
#include "netlink_comm.h"

static int codel_parse_stats(struct nlattr *attr, struct recordset *rset)
{
	struct tc_codel_xstats *st;

	if(nla_len(attr) < sizeof(*st))
		return -1;

	st = nla_data(attr);

	add_record_uint(rset, "maxpacket", sizeof("maxpacket"), st->maxpacket);
	add_record_uint(rset, "drop_count", sizeof("drop_count"), st->count);
	add_record_uint(rset, "drop_lastcount", sizeof("drop_lastcount"), st->lastcount);
	add_record_uint(rset, "drop_overlimit", sizeof("drop_overlimit"), st->drop_overlimit);
	add_record_uint(rset, "ecn_mark", sizeof("ecn_mark"), st->ecn_mark);
	add_record_uint(rset, "ldelay", sizeof("ldelay"), st->ldelay);
	add_record_uint(rset, "drop_next", sizeof("drop_next"), st->drop_next);
	add_record_uint(rset, "dropping", sizeof("dropping"), st->dropping);

	return 0;
}

struct qdisc_handler codel_qdisc_handler = {
	.id = "codel",
	.parse_stats = codel_parse_stats,
};
