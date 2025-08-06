#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "json.h"

struct json_key {
	char *key;
	struct json_value value;
};

struct json_parser {
	char *current;
	int line_number;
	int column;
};

void next(struct json_parser *p);
char peek(struct json_parser *p);

void json_array_add(struct json_value *array, struct json_value child)
{
	if (array->length >= array->max) {
		int n = array->length * 2;
		array->children = realloc(array->children, n * sizeof(struct json_value));
		array->max = n;
	}

	array->children[array->length++] = child;
}

void json_object_add(struct json_value *object, char *key, struct json_value value)
{
	if (object->length >= object->max) {
		int n = object->length * 2 + 1;
		object->keys = realloc(object->keys, n * sizeof(struct json_key));
		object->max = n;
	}

	object->keys[object->length++] = (struct json_key) {
		.key = key,
		.value = value
	};
}

struct json_value json_array_get(struct json_value array, int n)
{
	return n < array.length ? array.children[n] : json_undefined;
}

struct json_value json_object_get(struct json_value object, char *key)
{
	for (int i = 0; i < object.length; i++) {
		if (strcmp(object.keys[i].key, key) == 0) {
			return object.keys[i].value;
		}
	}

	return json_undefined;
}

void json_free(struct json_value value)
{
	switch (value.type) {
	case json_type_undefined:
	case json_type_null:
	case json_type_true:
	case json_type_false:
	case json_type_number:
		return;
	case json_type_string:
		free(value.string);
		break;
	case json_type_array:
		for (int i = 0; i < value.length; i++) {
			json_free(value.children[i]);
		}

		free(value.children);
		break;
	case json_type_object:
		for (int i = 0; i < value.length; i++) {
			free(value.keys[i].key);
			json_free(value.keys[i].value);
		}

		free(value.keys);
		break;
	default:
		assert(false);
	}
}

void append(char **s, const char *fmt, ...)
{
	int len, n;
	va_list vl;

	len = strlen(*s);

	va_start(vl, fmt);
	n = vsnprintf(NULL, 0, fmt, vl);
	va_end(vl);

	*s = realloc(*s, len + n + 1);

	va_start(vl, fmt);
	vsnprintf(*s + len, n + 1, fmt, vl);
	va_end(vl);
}

char *json_sprintf(char *fmt, ...)
{
	va_list vl;

	va_start(vl, fmt);
	int len = vsnprintf(NULL, 0, fmt, vl);
	if (len < 0) {
		return NULL;
	}
	va_end(vl);

	char *s = malloc(len + 1);
	if (s == NULL) {
		return NULL;
	}

	va_start(vl, fmt);
	if (vsnprintf(s, len + 1, fmt, vl) < 0) {
		return NULL;
	}
	va_end(vl);

	return s;
}

void skip_spaces(struct json_parser *parser)
{
	while (isspace(peek(parser))) {
		next(parser);
	}
}

char *stringify_key_value(struct json_key key)
{
	return json_sprintf("\"%s\": %s", key.key, json_stringify(key.value));
}

char *json_stringify(struct json_value v)
{
	char *children = strdup("");

	switch (v.type) {
	case json_type_array:
		for (int i = 0; i < v.length - 1; i++) {
			append(&children, "%s, ", json_stringify(v.children[i]));
		}

		if (v.length > 0) {
			append(&children, "%s", json_stringify(v.children[v.length - 1]));
		}

		return json_sprintf("[%s]", children);
	case json_type_object:
		for (int i = 0; i < v.length - 1; i++) {
			append(&children, "%s, ", stringify_key_value(v.keys[i]));
		}

		if (v.length > 0) {
			append(&children, "%s", stringify_key_value(v.keys[v.length - 1]));
		}

		return json_sprintf("{%s}", children);
	case json_type_number:
		return json_sprintf("%i", v.number);
	case json_type_string:
		return json_sprintf("\"%s\"", v.string);
	case json_type_true:
		return json_sprintf("true");
	case json_type_false:
		return json_sprintf("false");
	case json_type_null:
		return json_sprintf("null");
	case json_type_undefined:
		return json_sprintf("undefined");
	default:
		assert(false);
	}
}

bool json_equal(struct json_value a, struct json_value b)
{
	if (a.type != b.type) {
		return false;
	}

	switch (a.type) {
	case json_type_number:
		return a.number == b.number;
	case json_type_string:
		return a.string == b.string || !strcmp(a.string, b.string);
	case json_type_array:
		if (a.length != b.length) {
			return false;
		}

		for (int i = 0; i < a.length; i++) {
			if (!json_equal(a.children[i], b.children[i])) {
				return false;
			}
		}

		return true;
	case json_type_object:
		if (a.length != b.length) {
			return false;
		}

		for (int i = 0; i < a.length; i++) {
			struct json_value want = a.keys[i].value;
			struct json_value got = json_object_get(b, a.keys[i].key);

			if (!json_equal(want, got)) {
				return false;
			}
		}

		return true;
	case json_type_true:
	case json_type_false:
	case json_type_null:
		return true;
	default:
		assert(false);
	}
}

static struct json_parse_result error(struct json_parser *parser, char *fmt, ...)
{
	va_list vl;

	va_start(vl);
	char *err = json_sprintf(fmt, vl);
	va_end(vl);

	return (struct json_parse_result) {
		.value = json_undefined,
		.error = err,
		.line_number = parser->line_number,
		.column = parser->column
	};
}

struct json_value json_number(int n)
{
	return (struct json_value) {
		.type = json_type_number,
		.number = n
	};
}

struct json_value json_string(char *s)
{
	return (struct json_value) {
		.type = json_type_string,
		.string = s
	};
}

struct json_value json_array(void)
{
	return (struct json_value) {
		.type = json_type_array,
		.length = 0,
		.children = NULL
	};
}

struct json_value json_object(void)
{
	return (struct json_value) {
		.type = json_type_object,
		.length = 0,
		.keys = NULL
	};
}

bool awaits(struct json_parser *parser, char *s)
{
	int len, i;

	len = strlen(s);
	if (strncmp(parser->current, s, len) != 0) {
		return false;
	}

	if (isalnum(parser->current[len])) {
		/* still inside a symbol */
		return false;
	}

	for (i = 0; i < len; i++) {
		next(parser);
	}

	return true;
}

struct json_parse_result ok(struct json_value value)
{
	return (struct json_parse_result) {
		.value = value,
		.error = NULL,
		.line_number = -1,
		.column = -1,
	};
}

void next(struct json_parser *parser)
{
	if (peek(parser) == '\0') {
		return;
	}

	if (peek(parser) == '\n' || peek(parser) == '\r') {
		parser->line_number++;
		parser->column = 0;
	} else {
		parser->column++;
	}

	parser->current++;
}

char peek(struct json_parser *parser)
{
	return *parser->current;
}

bool consume(struct json_parser *parser, char c)
{
	if (peek(parser) == c) {
		next(parser);
		return true;
	} else {
		return false;
	}
}

struct json_parse_result json_parse_string(struct json_parser *parser)
{
	char *start, *copy;

	start = parser->current;
	while (!consume(parser, '"')) {
		if (peek(parser) == '\0') {
			return error(parser, "unterminated string");
		}
		next(parser);
	}

	copy = strndup(start, parser->current - start - 1);
	
	return ok(json_string(copy));
}

char *json_parse_symbol(struct json_parser *parser)
{
	char *start;

	start = parser->current;

	while (isalnum(peek(parser))) {
		next(parser);
	}

	return strndup(start, parser->current - start);
}

struct json_parse_result json_parse_value(struct json_parser *parser)
{
	if (consume(parser, '"')) {
		return json_parse_string(parser);
	} else if (consume(parser, '[')) {
		struct json_value array = json_array();
		bool can_terminate = true;

		for (;;) {
			skip_spaces(parser);

			if (peek(parser) == ']') {
				if (!can_terminate) {
					return error(parser, "Unexpected termination in list.");
				}

				next(parser);
				return ok(array);
			}

			can_terminate = false;

			struct json_parse_result child = json_parse_value(parser);
			if (child.error != NULL) {
				return child;
			}

			json_array_add(&array, child.value);

			can_terminate = !consume(parser, ',');
		}

	} else if (consume(parser, '{')) {
		struct json_value object = json_object();
		bool can_terminate = true;

		for (;;) {
			skip_spaces(parser);

			if (peek(parser) == '}') {
				if (!can_terminate) {
					return error(parser, "Can't terminate object after comma");
				}

				next(parser);
				return ok(object);
			}

			char *key;

			if (consume(parser, '"')) {
				struct json_parse_result tmp = json_parse_string(parser);
				if (tmp.error) {
					return tmp;
				}
				key = tmp.value.string;
			} else if (isalnum(peek(parser))) {
				key = json_parse_symbol(parser);
			} else {
				return error(parser, "Illegal character: '%c'", peek(parser));
			}

			if (!consume(parser, ':')) {
				return error(parser, "Expected ':' after object, got '%c'", peek(parser));
			}

			skip_spaces(parser);

			struct json_parse_result tmp = json_parse_value(parser);
			if (tmp.error) {
				return tmp;
			}
			json_object_add(&object, key, tmp.value);

			can_terminate = !consume(parser, ',');
		}

	} else if (awaits(parser, "true")) {
		return ok(json_true);
	} else if (awaits(parser, "false")) {
		return ok(json_false);
	} else if (awaits(parser, "null")) {
		return ok(json_null);
	} else if (isdigit(peek(parser))) { 
		int n = 0;

		while (isdigit(peek(parser))) {
			n *= 10;
			n += peek(parser) - '0';
			next(parser);
		}

		return ok(json_number(n));
	} else {
		return error(parser, "Illegal character: '%c'", peek(parser));
	}
}

struct json_parse_result json_parse(char *s)
{
	struct json_parser parser;
	struct json_parse_result value;

	parser.current = s;
	parser.line_number = 0;
	parser.column = 0;

	skip_spaces(&parser);

	value = json_parse_value(&parser);
	if (value.error != NULL) {
		return value;
	}

	skip_spaces(&parser);

	if (peek(&parser) != '\0') {
		return error(&parser, "Illegal character: '%c'. Expected EOF", peek(&parser));
	}

	return value;
}
