#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "json.h"

static char *json_sprintf(char *fmt, ...)
{
	char *s = NULL;
	va_list vl;

	va_start(vl);
	int needed = vsnprintf(s, 0, fmt, vl) + 1;
	s = malloc(needed);
	vsnprintf(s, needed, fmt, vl);
	va_end(vl);

	return s;
}

char *json_value_stringify(struct json_value v)
{
	switch (v.type) {
	case json_number:
		return json_sprintf("%i", v.number);
	case json_string:
		return json_sprintf("\"%s\"", v.string);
	default:
		assert(false);
	}
}

bool json_values_equal(struct json_value a, struct json_value b)
{
	if (a.type != b.type) {
		return false;
	}

	switch (a.type) {
	case json_number:
		return a.number == b.number;
	case json_string:
		return a.string == b.string || !strcmp(a.string, b.string);
	default:
		assert(false);
	}
}

static struct json_parse_result error(char *fmt, ...)
{
	struct json_parse_result result;
	va_list vl;
	int nbytes;

	va_start(vl);
	nbytes = vsnprintf(result.error, 0, fmt, vl);
	result.error = malloc(nbytes);
	vsnprintf(result.error, nbytes, fmt, vl);
	va_end(vl);

	return result;
}

struct json_value json_value_number(int n)
{
	return (struct json_value) {
		.type = json_number,
		.number = n
	};
}

struct json_value json_value_string(const char *s)
{
	return (struct json_value) {
		.type = json_string,
		.string = s
	};
}

struct json_parse_result json_parse(const char *s)
{
	struct json_value result;
	const char *p;
	int len, n;
	enum {
		state_start,
		state_number,
		state_string,
		state_end
	} state;

	state = state_start;

	for (;;) {
		switch (state) {
		case state_start:
			if (isspace(*s)) {
				s++;
				continue;
			}

			if (isdigit(*s)) {
				state = state_number;
				n = 0;
				continue;
			}

			if (*s == '"') {
				len = 0;
				p = s++;
				state = state_string;
				continue;
			}

			return error("Illegal character: '%c'", *s);

		case state_number:
			if (!isdigit(*s)) {
				state = state_end;
				result = json_value_number(n);
				continue;
			}

			n *= 10;
			n += (*s++ - '0');

			break;

		case state_string:
			if (*s++ == '"') {
				result = json_value_string(strndup(p, len));
				state = state_end;
				continue;
			}

			len++;
			
			break;

		case state_end:
			if (*s == '\0') {
				return (struct json_parse_result) { .value = result };
			}
			
			if (isspace(*s)) {
				s++;
				continue;
			}

			return error("Illegal character: '%c'", *s);

		default:
			assert(false);
		}
	}
}
