/**
 * csv_formatter.c
 *
 * Toke Høiland-Jørgensen
 * 2013-06-23
 */

#include <string.h>

#include "formatter.h"

int create_header(char *buf, unsigned int maxlen, struct recordset *rset)
{
	struct record *r;
	unsigned int pos = 0, len;
	for_each_record(r, rset) {
		len = strlen(r->name);
		if(pos+len+2 > maxlen)
			goto err;
		memcpy(buf+pos, r->name, len);
		pos += len;
		if(r->next)
			buf[pos++] = ',';
	}
	buf[pos] = '\0';
	return 0;
err:
	buf[pos] = '\0';
	return -1;
}

int csv_format(struct formatter *fmt, struct recordset *rset)
{
	struct record *r;
	static char header[256];
	char buf[256];
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
		fputs(r->value, fmt->f);
		if(r->next)
			fputc(',', fmt->f);
	}
	fputc('\n', fmt->f);
}


struct formatter csv_formatter = {
	.id = "csv",
	.format = csv_format,
};
