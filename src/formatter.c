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
#include "util.h"

static struct formatter *formatter_list;

struct formatter *find_formatter(const char *name)
{
	struct formatter *fmt;
	char buf[128] = {0};
	void *dlh;
	int ret;

	for(fmt = formatter_list; fmt; fmt=fmt->next)
		if(strcmp(fmt->id, name) == 0)
			return fmt;

	if((ret = snprintf(buf, sizeof(buf), "%s_formatter", name)) > sizeof(buf)-1) {
		fprintf(stderr, "Warning: Ran out of buffer space while looking for formatter.\n");
		return NULL;
	}
	dlh = dlopen(NULL, RTLD_LAZY);
	fmt = dlsym(dlh, buf);
	if(fmt) {
		if(fmt->init && fmt->init(fmt) != 0) {
			fprintf(stderr, "Error during formatter init.\n");
			if(fmt->priv)
				free(fmt->priv);
			return NULL;
		}
		fmt->next = formatter_list;
		formatter_list = fmt;
	}
	return fmt;
}

/**
 * Allocate a new record structure.
 *
 * Memory is allocated with len_n + len_v space at the end of the structure.
 *
 * If len_v > 0, the value pointer is initialised to point to the memory
 * just after the structure.
 *
 * The record name is recorded at the end of the allocated block, after the
 * structure itself and the (optional) space for the value (as specified in len_v).
 *
 **/
struct record *alloc_record(const char *name, size_t len_n, const void *value, size_t len_v)
{
	struct record *r;
	int len = len_n + len_v;
	r = malloc(sizeof(*r) + len);
	if(!r)
		return NULL;
	memset(r, 0, len);
	r->next = NULL;
	r->name = (char *)r + sizeof(*r) + len_v;
	r->len_n = len_n;
	r->len_v = len_v;
	memcpy(r->name, name, len_n);


	if(len_v)
		r->value = (void *)r + sizeof(*r);
	if(value)
		memcpy(r->value, value, len_v);

	return r;
}

void clear_records(struct recordset *rset)
{
	struct record *cur, *next;
	cur = rset->records;
	while(cur) {
		next = cur->next;
		destroy_record(cur);
		cur = next;
	}
}

void destroy_record(struct record *r)
{
	if(r->type == RECORD_TYPE_RSET) {
		clear_records(r->value_rset);
	}
	free(r);
}

struct record *add_record(struct recordset *rset, struct record *r)
{
	struct record *cur;
	cur = rset->records;
	if(!cur) {
		rset->records = r;
	} else {
		while(cur->next)
			cur = cur->next;
		cur->next = r;
	}
	rset->len++;
	return r;
}

struct record *add_record_str(struct recordset *rset,
			const char *name, size_t len_n,
			const char *value, size_t len_v)
{
	struct record *r;

	r = alloc_record(name, len_n, value, len_v);
	if(!r)
		return NULL;

	r->type = RECORD_TYPE_STRING;

	return add_record(rset, r);
}

struct record *add_record_int(struct recordset *rset, const char *name, size_t len_n, int value)
{
	struct record *r;
	r = alloc_record(name, len_n, NULL, 0);
	if(!r)
		return NULL;
	r->type = RECORD_TYPE_INT;
	r->value_int = value;

	return add_record(rset, r);
}

struct record *add_record_uint(struct recordset *rset, const char *name, size_t len_n, unsigned int value)
{
	struct record *r;
	r = alloc_record(name, len_n, NULL, 0);
	if(!r)
		return NULL;
	r->type = RECORD_TYPE_UINT;
	r->value_uint = value;

	return add_record(rset, r);
}

struct record *add_record_hex(struct recordset *rset, const char *name, size_t len_n, unsigned int value)
{
	struct record *r = add_record_uint(rset, name, len_n, value);
	r->type = RECORD_TYPE_HEX;
	return r;
}

struct record *add_record_double(struct recordset *rset, const char *name, size_t len_n, double value)
{
	struct record *r;
	r = alloc_record(name, len_n, NULL, 0);
	if(!r)
		return NULL;
	r->type = RECORD_TYPE_DOUBLE;
	r->value_double = value;

	return add_record(rset, r);
}

struct record *add_record_rset(struct recordset *rset,
			const char *name, size_t len_n,
			const struct recordset *value)
{
	struct record *r;
	r = alloc_record(name, len_n, value, sizeof(struct recordset));
	if(!r)
		return NULL;
	r->type = RECORD_TYPE_RSET;

	return add_record(rset, r);
}

int record_format_value(char *buf, size_t len, const struct record *r)
{
	switch(r->type) {
	case RECORD_TYPE_STRING:
		if(r->len_v > len) {
			memcpy(buf, r->value_str, len);
			buf[len] = '\0';
		} else {
			memcpy(buf, r->value_str, r->len_v);
		}
		return r->len_v;
		break;
	case RECORD_TYPE_INT:
		return 1 + snprintf(buf, len, "%d", r->value_int);
		break;
	case RECORD_TYPE_UINT:
		return 1 + snprintf(buf, len, "%u", r->value_uint);
		break;
	case RECORD_TYPE_HEX:
		return 1 + snprintf(buf, len, "%x", r->value_uint);
		break;
	case RECORD_TYPE_DOUBLE:
		return 1 + snprintf(buf, len, "%f", r->value_double);
		break;
	default:
		buf[0] = '\0';
		return 0;
	}
}



static int null_format(struct formatter *fmt, struct recordset *records)
{
	return 0;
}

struct formatter null_formatter = {
	.id = "null",
	.format = null_format,
};

static int print_format(struct formatter *fmt, struct recordset *rset)
{
	struct record *r;
	unsigned int width = 0;
	int len, len_v;
	char buf[128] = {0};
	for_each_record(r, rset) {
		len_v = record_format_value(buf, sizeof(buf), r);
		len = r->len_n + min(sizeof(buf), len_v) + 1;
		if(width + len > OUTPUT_WIDTH) {
			fputs("\n  ", fmt->f);
			width = 2;
		}
		width += len;
		fputs(r->name, fmt->f);
		fputs(": ", fmt->f);
		fputs(buf, fmt->f);
		fputc(' ', fmt->f);
		if(r->type == RECORD_TYPE_RSET) {
			fputs("\n    ", fmt->f);
			print_format(fmt, r->value_rset);
			width = 0;
		}
	}
	if(width > 0)
		fputc('\n', fmt->f);
	return 0;
}

struct formatter print_formatter = {
	.id = "print",
	.format = print_format,
};
