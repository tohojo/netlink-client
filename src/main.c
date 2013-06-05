/**
 * main.c
 *
 * Toke Høiland-Jørgensen
 * 2013-06-03
 */

#include <stdio.h>

#include "netlink_comm.h"
#include "options.h"

int main(int argc, char **argv)
{
	int ret;
	struct options opt;

	printf("Initialising...\n");

	if(initialise_options(&opt, argc, argv) < 0) {
		fprintf(stderr, "Unable to initialise options.\n");
		return 1;
	}

	printf("Netlink socket set up...\n");

	while(1)
		nl_recvmsgs_default(opt.sk_listen);

}
