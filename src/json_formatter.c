/**
 * json_formatter.c
 *
 * Toke Høiland-Jørgensen
 * 2013-06-24
 */

#include <unistd.h>
#include <string.h>
#include <yajl/yajl_gen.h>

#include "formatter.h"

struct json_formatter_data {
	yajl_gen yajl;
};

static void json_print(FILE *f, yajl_gen yajl)
{
	const unsigned char *buf;
	size_t len;
	yajl_gen_get_buf(yajl, &buf, &len);
	fputs((char*)buf, f);
	fputc('\n', f);
	yajl_gen_clear(yajl);
}

static int json_init(struct formatter *fmt)
{
	struct json_formatter_data *priv = fmt->priv;
	if((priv->yajl = yajl_gen_alloc(NULL)) == NULL)
		return -1;
	yajl_gen_array_open(priv->yajl);
	return 0;
}

static int json_format(struct formatter *fmt, struct recordset *rset)
{
	struct record *r;
	struct json_formatter_data *priv = fmt->priv;
	yajl_gen_map_open(priv->yajl);
	for_each_record(r, rset) {
		yajl_gen_string(priv->yajl, (unsigned char*)r->name, r->len_n-1);

		switch(r->type) {
		case RECORD_TYPE_STRING:
			yajl_gen_string(priv->yajl, (unsigned char*)r->value_str, r->len_v-1);
			break;
		case RECORD_TYPE_INT:
			yajl_gen_integer(priv->yajl, r->value_int);
			break;
		case RECORD_TYPE_UINT:
		case RECORD_TYPE_HEX:
			yajl_gen_integer(priv->yajl, r->value_uint);
			break;
		default:
			break;
		}
	}
	yajl_gen_map_close(priv->yajl);
	json_print(fmt->f, priv->yajl);
	return 0;
}

static void json_destroy(struct formatter *fmt)
{
	struct json_formatter_data *priv = fmt->priv;
	yajl_gen_array_close(priv->yajl);
	json_print(fmt->f, priv->yajl);
	yajl_gen_free(priv->yajl);
}

static struct json_formatter_data json_priv = {0};

struct formatter json_formatter = {
	.id = "json",
	.init = json_init,
	.destroy = json_destroy,
	.format = json_format,
	.priv = &json_priv,
};
