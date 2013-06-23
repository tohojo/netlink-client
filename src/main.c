/**
 * main.c
 *
 * Toke Høiland-Jørgensen
 * 2013-06-03
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "options.h"

static struct options opt;

static void sig_exit(int signal)
{
	destroy_options(&opt);
	exit(signal);
}

struct sigaction sigact = {
	.sa_handler = sig_exit,
	.sa_mask = 0,
	.sa_flags = 0,
};

int main(int argc, char **argv)
{
	int ret;

	if(sigaction(SIGINT, &sigact, NULL) < 0 ||
		sigaction(SIGTERM, &sigact, NULL) < 0) {
		perror("Error installing signal handler");
		return 1;
	}

	if(initialise_options(&opt, argc, argv) < 0)
		return 1;

	printf("Netlink socket set up...\n");

	nl_recvmsgs_default(opt.sk_listen);
	destroy_options(&opt);
	return 0;

}
