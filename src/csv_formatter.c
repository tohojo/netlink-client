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
	unsigned int pos = 0;
	for_each_record(r, rset) {
		pos += snprintf(buf+pos, maxlen-pos, "%s", r->name);
		if(r->next)
			pos += snprintf(buf+pos, maxlen-pos, ",");
		if(pos >= maxlen)
			return -1;
	}
	return 0;
}

int csv_format(struct formatter *fmt, struct recordset *rset)
{
	struct record *r;
	static char header[255];
	char buf[255];
	create_header(buf, sizeof(buf), rset);
	if(strcmp(header, buf) != 0) {
		strncpy(header, buf, sizeof(header));
		fputs(header, fmt->f);
		fputs("\n", fmt->f);
	}
	for_each_record(r, rset){
		fputs(r->value, fmt->f);
		if(r->next)
			fputs(",", fmt->f);
	}
	fputs("\n", fmt->f);
}


struct formatter csv_formatter = {
	.id = "csv",
	.format = csv_format,
};
