/**
 * formatter.h
 *
 * Toke Høiland-Jørgensen
 * 2013-06-23
 */

#ifndef FORMATTER_H
#define FORMATTER_H

#include <stdio.h>

#define DEFAULT_FORMATTER "print"
#define OUTPUT_WIDTH 80
#define RECORD_FIELDLENGTH 100

struct record {
	struct record *next;
	char name[RECORD_FIELDLENGTH];
	char value [RECORD_FIELDLENGTH];
	char composite;
};

struct recordset {
	unsigned int length;
	struct record *records;
};

struct formatter {
	struct formatter *next;
	const char *id;
	FILE *f;
	int (*init)(struct formatter *formatter);
	int (*format)(struct formatter *formatter, struct recordset *records);
	int priv_size;
	void *priv;
};

struct formatter *find_formatter(const char *name);

int clear_records(struct recordset *rset);
struct record *add_record(struct recordset *rset, const char *name, const char *value);
struct record *add_crecord(struct recordset *rset, const char *name, const char *value);
struct record *add_record_u(struct recordset *rset, const char *name, unsigned int value);

#define for_each_record(r, rset) for(r = rset->records; r; r = r->next)


#endif
