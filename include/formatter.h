/**
 * formatter.h
 *
 * Toke Høiland-Jørgensen
 * 2013-06-23
 */

#ifndef FORMATTER_H
#define FORMATTER_H

#include <stdio.h>

struct recordset {
	unsigned int length;
	struct record *records;
};

struct record {
	char name[100];
	char value [100];
};

struct formatter {
	const char *id;
	FILE *f;
	int (*format)(struct formatter *formatter, struct recordset *records);
};

struct formatter *find_formatter(const char *name);

#define DEFAULT_FORMATTER "print"

#endif
