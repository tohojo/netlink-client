/**
 * csv_formatter.c
 *
 * Toke Høiland-Jørgensen
 * 2013-06-23
 */

#include "formatter.h"

int csv_format(struct formatter *fmt, struct recordset *rset)
{
	struct record *r;
	static char first = 1;
	if(first) {
		first = 0;
		for_each_record(r, rset)
			if(!r->composite) {
				fputs(r->name, fmt->f);
				if(r->next)
					fputs(",", fmt->f);
			}
		fprintf(fmt->f, "\n");
	}
	for_each_record(r, rset)
		if(!r->composite) {
			fputs(r->value, fmt->f);
			if(r->next)
				fputs(",", fmt->f);
		}
	fprintf(fmt->f, "\n");
}

struct formatter csv_formatter = {
	.id = "csv",
	.format = csv_format,
};
