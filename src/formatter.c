/**
 * formatter.c
 *
 * Toke Høiland-Jørgensen
 * 2013-06-23
 */
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "formatter.h"

static struct formatter *formatter_list;

struct formatter *find_formatter(const char *name)
{
	struct formatter *fmt;
	char buf[128];
	void *dlh;

	for(fmt = formatter_list; fmt; fmt=fmt->next)
		if(strcmp(fmt->id, name) == 0)
			return fmt;

	dlh = dlopen(NULL, RTLD_LAZY);
	snprintf(buf, sizeof(buf), "%s_formatter", name);
	fmt = dlsym(dlh, buf);
	if(fmt) {
		fmt->next = formatter_list;
		formatter_list = fmt;
	}
	return fmt;
}

struct record *add_record(struct recordset *rset, const char *name, const char *value)
{
	struct record *r, *cur;
	unsigned int len_n, len_v;
	r = malloc(sizeof(*r));
	memset(r, 0, sizeof(*r));
	if(!r)
		return NULL;
	if((len_n = strlen(name)) > RECORD_FIELDLENGTH-1 ||
		(len_v = strlen(value)) > RECORD_FIELDLENGTH-1)
		goto err;

	memcpy(r->name, name, len_n+1);
	memcpy(r->value, value, len_v+1);

	cur = rset->records;
	if(!cur) {
		rset->records = r;
	} else {
		while(cur->next)
			cur = cur->next;
		cur->next = r;
	}
	rset->length++;

	return r;
err:
	free(r);
	return NULL;
}

struct record *add_crecord(struct recordset *rset, const char *name, const char *value)
{
	struct record *r = add_record(rset, name, value);
	if(r)
		r->composite = 1;
	return r;
}

struct record *add_record_u(struct recordset *rset, const char *name, unsigned int value)
{
	char buf[128];
	snprintf(buf, sizeof(buf), "%u", value);
	return add_record(rset, name, buf);
}

int clear_records(struct recordset *rset)
{
	struct record *cur, *next;
	cur = rset->records;
	while(cur) {
		next = cur->next;
		free(cur);
		cur = next;
	}
	return 0;
}


int null_format(struct formatter *fmt, struct recordset *records)
{
	return 0;
}

struct formatter null_formatter = {
	.id = "null",
	.format = null_format,
};

int print_format(struct formatter *fmt, struct recordset *rset)
{
	struct record *r;
	unsigned int width = 0, buflen;
	char buf[128];
	for_each_record(r, rset) {
		snprintf(buf, sizeof(buf), "%s: %s ", r->name, r->value);
		width += strnlen(buf, sizeof(buf));
		if(width > OUTPUT_WIDTH || r->composite) {
			fputs("\n  ", fmt->f);
			width = 2;
		}
		fputs(buf, fmt->f);
	}
	if(width > 0)
		fputc('\n', fmt->f);
}

struct formatter print_formatter = {
	.id = "print",
	.format = print_format,
};
