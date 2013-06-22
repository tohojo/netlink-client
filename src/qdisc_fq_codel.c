/**
 * qdisc_fq_codel.c
 *
 * Toke Høiland-Jørgensen
 * 2013-06-22
 */

#include "netlink_comm.h"

static int fq_codel_print_stats(struct nlattr *attr)
{
	printf("fq_codel print!\n");
}

struct qdisc_handler fq_codel_qdisc_handler = {
	.id = "fq_codel",
	.print_stats = fq_codel_print_stats,
};
