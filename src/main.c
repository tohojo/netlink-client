/**
 * main.c
 *
 * Toke Høiland-Jørgensen
 * 2013-06-03
 */

#include <stdio.h>

#include "netlink_comm.h"

int main()
{
	int ret;

	printf("Initialising...\n");

	struct nl_sock *sk = create_socket();
	if(sk == NULL) {
		fprintf(stderr, "Unable to create netlink socket.\n", sk);
		return 1;
	}
	if((ret = setup_socket(sk)) < 0) {
		destroy_socket(sk);
		fprintf(stderr, "Error setting up socket: %d.\n", -ret);
		return 1;
	}


	printf("Netlink socket set up...\n");

	while(1)
		nl_recvmsgs_default(sk);

}
