/**
 * csv_formatter.c
 *
 * Toke Høiland-Jørgensen
 * 2013-06-23
 */

#include <string.h>

#include "formatter.h"

static int create_header(char *buf, size_t maxlen, struct recordset *rset)
{
	struct record *r;
	unsigned int pos = 0;
	for_each_record(r, rset) {
		if(pos+r->len_n+1 > maxlen)
			goto err;
		memcpy(buf+pos, r->name, r->len_n-1);
		pos += r->len_n-1;
		if(r->next)
			buf[pos++] = ',';
	}
	buf[pos] = '\0';
	return 0;
err:
	buf[pos] = '\0';
	return -1;
}

static int csv_format(struct formatter *fmt, struct recordset *rset)
{
	struct record *r;
	static char header[256] = {0};
	char buf[256] = {0};
	if(create_header(buf, sizeof(buf), rset) < 0) {
		fprintf(stderr, "Warning: Ran out of buffer space "
			"while constructing csv header.\n");
	} else {
		if(strcmp(header, buf) != 0) {
			memcpy(header, buf, sizeof(header));
			fputs(header, fmt->f);
			fputc('\n', fmt->f);
		}
	}
	for_each_record(r, rset){
		record_format_value(buf, sizeof(buf), r);
		fputs(buf, fmt->f);
		if(r->next)
			fputc(',', fmt->f);
	}
	fputc('\n', fmt->f);
	return 0;
}


struct formatter csv_formatter = {
	.id = "csv",
	.format = csv_format,
};
