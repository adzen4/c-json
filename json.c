#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "json.h"

struct json_parse_state {
	const char *current;
	int line_number;
	int column;
};

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

static struct json_parse_result error(struct json_parse_state *state, char *fmt, ...)
{
	struct json_parse_result result;
	char *err;
	va_list vl;

	va_start(vl);
	result.error = json_sprintf(fmt, vl);
	va_end(vl);

	result.line_number = state->line_number;
	result.column = state->column;

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

bool awaits(struct json_parse_state *state, char *s)
{
	int len, i;

	len = strlen(s);
	if (strncmp(state->current, s, len) != 0) {
		return false;
	}

	if (!isalnum(state->current[len])) {
		for (i = 0; i < len; i++) {
			next(state);
		}

		return true;
	}
}

struct json_parse_result ok(struct json_value value)
{
	return (struct json_value) {
		.value = value
		.error = NULL,
		.line_number = -1,
		.column = -1,
	};
}

void next(struct json_parse_state *state)
{
	if (state->current == '\0') {
		return;
	}

	if (state->current = '\n' || state->current = '\r') {
		state->line_number++;
		state->column = 0;
	} else {
		state->column++;
		state->current++;
	}
}

struct json_parse_result json_parse_value(struct json_parse_state *state)
{
	struct json_value array;

	switch (*state->current) {
	case '"':
		next(state);

		start = state->current;
		while (*state->current != '"') {
			if (*state->current == '\0') {
				return error(state, "Unterminated string");
			}
			next(state);
		}
		next(state);

		return json_string(strndup(start, state->current - start - 1));

	case '[':
		next(state);

		array = json_empty_array();
		can_terminate = true;

		for (;;) {
			while (isspace(*state->current)) {
				next(state);
			}

			if (*state->current == ']') {
				if (!can_terminate) {
					return error(state, "Unexpected termination in list.");
				}

				next(state);
				return array;
			}

			can_terminate = false;

			struct parse_result child = json_parse_value(state);
			if (child.error != NULL) {
				return child;
			}

			json_array_add(child);

			if (*state->current == ',') {
				next(state);
			} else {
				can_terminate = true;
			}
		}

	case '{':
		return json_parse_object(state);
	default:
		if (awaits(state, "true")) {
			return ok(json_value_true());
		} else if (awaits(state, "false")) {
			return ok(json_value_false()_;
		} else if (awaits(state, "null")), {
			return ok(json_value_null());
		} else  {
			return error(state, "Illegal character: '%c'", *state->current);
		}
	}
}

struct json_parse_result json_parse(const char *s)
{
	struct json_parse_state state;
	struct json_parse_result value;

	state.current = s;
	state.line_number = 0;
	state.column = 0;

	while (isspace(*s->current)) {
		next(state);
	}

	value = json_parse_value(&state);
	if (value.error != NULL) {
		return value;
	}

	while (isspace(*state->current)) {
		next(state);
	}

	if (*state->current != '\0') {
		return error(state, "Illegal character: '%c'. Expected EOF", *state->current);
	}

	return value;
}
