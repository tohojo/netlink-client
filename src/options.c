/**
 * options.c
 *
 * Toke Høiland-Jørgensen
 * 2013-06-04
 */

#include "options.h"
#include "netlink_comm.h"

int parse_options(struct options *opt, int argc, char **argv);


int initialise_options(struct options *opt, int argc, char **argv)
{
	int ret;
	opt->if_c = 0;
	opt->ifs = NULL;
	opt->run_length = 60;
	opt->formatter = find_formatter(DEFAULT_FORMATTER);
	opt->formatter->f = stdout;
	gettimeofday(&opt->start_time, NULL);

	opt->sk_req = create_socket();
	opt->sk_listen = create_socket();
	if(opt->sk_req == NULL || opt->sk_listen == NULL) {
		fprintf(stderr, "Unable to create netlink sockets.\n");
		goto err;
	}


	if((ret = setup_socket(opt->sk_listen, opt)) < 0) {
		fprintf(stderr, "Error setting up listen socket: %d.\n", -ret);
		goto err;
	}

	if((ret = nl_connect(opt->sk_req, NETLINK_ROUTE)) < 0) {
		fprintf(stderr, "Error setting up request socket: %d.\n", -ret);
		goto err;
	}


	if((ret = rtnl_link_alloc_cache(opt->sk_req, AF_UNSPEC, &opt->cache)) < 0) {
		fprintf(stderr, "Unable to allocate link cache: %d.\n", ret);
		goto err;
	}

	if(parse_options(opt, argc, argv) < 0)
		return -2;

	if(opt->if_c == 0)
		fprintf(stderr, "Warning: No interfaces selected, you won't see any output.\n");

	opt->initialised = 1;
	return 0;

err:
	if(opt->sk_req != NULL)
		destroy_socket(opt->sk_req);
	if(opt->sk_listen != NULL)
		destroy_socket(opt->sk_listen);
	return -1;

}

void destroy_options(struct options *opt)
{
	if(!opt->initialised)
		return;
	if(opt->formatter->destroy)
		opt->formatter->destroy(opt->formatter);
	if(opt->formatter->f != stdout)
		fclose(opt->formatter->f);
	destroy_socket(opt->sk_req);
	destroy_socket(opt->sk_listen);
	opt->initialised = 0;
}

int parse_options(struct options *opt, int argc, char **argv)
{
	int o;
	int ifid, val;
	FILE *output;
	struct formatter *formatter;

	while((o = getopt(argc, argv, "f:hi:l:o:")) != -1) {
		switch(o) {
		case 'f':
			formatter = find_formatter(optarg);
			if(formatter == NULL) {
				fprintf(stderr, "Unable to find formatter: %s.\n", optarg);
				return -1;
			}
			formatter->f = opt->formatter->f;
			opt->formatter = formatter;
			break;
		case 'i':
			ifid = rtnl_link_name2i(opt->cache, optarg);
			if(ifid == 0) {
				fprintf(stderr, "Unable to find interface: %s.\n", optarg);
				return -1;
			}
			if(add_iface(opt, ifid) < 0) {
				fprintf(stderr, "Couldn't add interface: %s.\n", optarg);
				return -1;
			}
			break;
		case 'l':
			val = atoi(optarg);
			if(val < 1) {
				fprintf(stderr, "Invalid length: %d\n", val);
				return -1;
			}
			opt->run_length = val;
			break;
		case 'o':
			if(formatter->f != stdout) {
				fprintf(stderr, "Output file already set.\n");
				return -1;
			}
			if(strcmp(optarg, "-") != 0) {
				output = fopen(optarg, "w");
				if(output == NULL) {
					perror("Unable to open output file");
					return -1;
				}
				formatter->f = output;
			}
			break;
		case 'h':
		default:
			fprintf(stderr, "Usage: %s [-i <ifname>] [-l <length>] [-f <print|csv|json>] [-o <outfile>]\n", argv[0]);
			return -1;
			break;
		}
	}
	return 0;
}

int add_iface(struct options *opt, int ifid)
{
	int *new_ifs;
	new_ifs = malloc((opt->if_c+1) * sizeof(int));
	if(new_ifs == NULL)
		return -1;
	if(opt->ifs != NULL) {
		memcpy(new_ifs, opt->ifs, opt->if_c * sizeof(int));
		free(opt->ifs);
	}
	opt->ifs = new_ifs;
	opt->ifs[opt->if_c++] = ifid;
	return 0;
}

int has_iface(struct options *opt, int ifid)
{
	int i;
	for(i = 0; i < opt->if_c; i++)
		if(opt->ifs[i] == ifid)
			return 1;
	return 0;
}
