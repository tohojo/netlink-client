/**
 * main.c
 *
 * Toke Høiland-Jørgensen
 * 2013-06-03
 */

#include <stdio.h>

#include "options.h"

int main(int argc, char **argv)
{
	int ret;
	struct options opt;

	if(initialise_options(&opt, argc, argv) < 0)
		return 1;

	printf("Netlink socket set up...\n");

	nl_recvmsgs_default(opt.sk_listen);
	return 0;

}
