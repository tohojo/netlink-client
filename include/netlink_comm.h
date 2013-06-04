/**
 * netlink_comm.h
 *
 * Toke Høiland-Jørgensen
 * 2013-06-04
 */

#ifndef NETLINK_COMM_H
#define NETLINK_COMM_H

#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/attr.h>
#include <netlink/msg.h>
#include <linux/gen_stats.h>

struct nl_sock *create_socket();
void destroy_socket(struct nl_sock *sk);
int setup_socket(struct nl_sock *sk);

static struct nla_policy tca_policy[TCA_MAX+1] = {
        [TCA_KIND] = { .type = NLA_STRING,
                          .maxlen = IFNAMSIZ },
        [TCA_STATS2] = { .type = NLA_NESTED },
};

static struct nla_policy tca_stats_policy[TCA_STATS_MAX+1] = {
  [TCA_STATS_BASIC] = { .minlen = sizeof(struct gnet_stats_basic)},
  [TCA_STATS_QUEUE] = { .minlen = sizeof(struct gnet_stats_queue)},
};



#endif
