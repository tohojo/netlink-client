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

typedef enum {
	RECORD_TYPE_UNDEF,
	RECORD_TYPE_STRING,
	RECORD_TYPE_INT,
	RECORD_TYPE_UINT,
	RECORD_TYPE_HEX,
	RECORD_TYPE_RSET,
} record_type_t;

struct record {
	struct record *next;
	record_type_t type;
	char *name;
	/* lengths are in bytes (so including the terminating \0 for strings) */
	size_t len_n;
	/* number of bytes allocated outside the struct (so 0 for (u)ints) */
	size_t len_v;
	union {
		void *value;
		char *value_str;
		int value_int;
		unsigned int value_uint;
		struct recordset *value_rset;
	};
};

struct recordset {
	size_t len;
	struct record *records;
};

struct formatter {
	struct formatter *next;
	const char *id;
	FILE *f;
	int (*init)(struct formatter *formatter);
	void (*destroy)(struct formatter *formatter);
	int (*format)(struct formatter *formatter, struct recordset *records);
	size_t priv_size;
	void *priv;
};

struct formatter *find_formatter(const char *name);

struct record *alloc_record(const char *name, size_t len_n, const void *value, size_t len_v);
void clear_records(struct recordset *rset);
void destroy_record(struct record *r);

struct record *add_record(struct recordset *rset, struct record *r);

struct record *add_record_str(struct recordset *rset,
			const char *name, size_t len_n,
			const char *value, size_t len_v);

struct record *add_record_int(struct recordset *rset,
			const char *name, size_t len_n,
			int value);

struct record *add_record_uint(struct recordset *rset,
			const char *name, size_t len_n,
			unsigned int value);

struct record *add_record_hex(struct recordset *rset,
			const char *name, size_t len_n,
			unsigned int value);

struct record *add_record_rset(struct recordset *rset,
			const char *name, size_t len_n,
			const struct recordset *value);

int record_format_value(char *buf, size_t len, const struct record *r);

#define for_each_record(r, rset) for(r = rset->records; r; r = r->next)


#endif
