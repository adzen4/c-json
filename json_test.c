#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "json.h"

static void test_json_basics(void)
{
	size_t i, len;
	const char *input;
	struct json_value want;
	struct json_parse_result got;
	struct {
		const char *input;
		struct json_value want;
	} tests[] = {
		{ "0", json_value_number(0) },
		{ "5", json_value_number(5) },
		{ "55", json_value_number(55) },
		{ "143", json_value_number(143) },
		{ "  143  ", json_value_number(143) },
		{ "\n\t143\n\n ", json_value_number(143) },

		/* strings */
		{ "\"\"", json_value_string("") }
	};
	
	len = sizeof tests / sizeof tests[0];

	for (i = 0; i < len; i++) {
		input = tests[i].input;
		want = tests[i].want;

		printf("\"%s\" should give <%s>: ", input, json_value_stringify(want));

		got = json_parse(input);

		if (got.error != NULL) {
			printf("got error: \"%s\"", got.error);
		} else if (!json_values_equal(want, got.value)) {
			printf("got <%s>", json_value_stringify(got.value));
		} else {
			printf("OK");
		}

		printf("\n");
	}
}

void test_json_values_equal()
{
	bool got, want;
	size_t i, len;
	const char *dummy_string = "hello";
	struct json_value a, b;
	struct {
		struct json_value a;
		struct json_value b;
		bool want;
	} tests[] = {
		{ json_value_number(10), json_value_number(10), true },
		{ json_value_number(10), json_value_number(11), false },
		{ json_value_number(10), json_value_string("hello"), false },
		{ json_value_number(0), json_value_string(NULL), false },
		{ json_value_string(dummy_string), json_value_string(strdup(dummy_string)), true },
	};

	len = sizeof tests / sizeof tests[0];

	for (i = 0; i < len; i++) {
		a = tests[i].a;
		b = tests[i].b;
		want = tests[i].want;
		
		printf("json_values_equal(%s, %s) should return %i: ",
		       json_value_stringify(a),
		       json_value_stringify(b),
		       want);

		got = json_values_equal(a, b);
		if (got != want) {
			printf("got %i", got);
		} else {
			printf("OK");
		}

		printf("\n");
	}
}

static struct {
	void (*fn)(void);
	char const *name;
} tests[] = {
	{ test_json_basics, "JSON Basics" },
	{ test_json_values_equal, "JSON Value Equality" }
};

int main(void)
{
	size_t i, len;

	len = sizeof tests / sizeof tests[0];

	for (i = 0; i < len; i++) {
		printf("=== TEST: %s\n", tests[i].name);
		tests[i].fn();
		printf("\n");
	}
}
