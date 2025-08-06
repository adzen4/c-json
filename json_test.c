#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"

void test_containers(void)
{
	struct json_value empty = json_array();

	if (empty.length != 0) {
		printf("empty array isn't empty");
	}

	if (empty.max != 0) {
		printf("empty array max isn't 0");
	}

	if (empty.children != NULL) {
		printf("empty array data isn't NULL");
	}

	json_array_add(&empty, json_true);

	if (empty.length != 1) {
		printf("Array length should be 1 after adding element\n");
	}

	struct json_value got = json_array_get(empty, 0);
	if (!json_equal(got, json_true)) {
		printf("Retrieved element is different to 'true': %s", json_stringify(got));
	}
}

void check(struct json_value value, char *want)
{
	char *got = json_stringify(value);
	if (strcmp(got, want) != 0) {
		printf("String is different: %s gives %s\n", want, got);
	}
}

struct json_value make_person(char *name, int age)
{
	struct json_value person = json_object();
	json_object_add(&person, "name", json_string(name));
	json_object_add(&person, "age", json_number(age));
	return person;
}

void test_stringify(void)
{
	struct json_value array = json_array();
	json_array_add(&array, json_true);

	check(array, "[true]");
	json_array_add(&array, json_false);
	check(array, "[true, false]");

	struct json_value object = json_object();
	check(object, "{}");
	json_object_add(&object, "a", json_true);
	check(object, "{\"a\": true}");

	struct json_value family = json_array();
	json_array_add(&family, make_person("David", 20));
	json_array_add(&family, make_person("Eve", 40));

	check(family, "[{\"name\": \"David\", \"age\": 20}, {\"name\": \"Eve\", \"age\": 40}]");
}

int main(void)
{
	test_containers();
	test_stringify();

	struct json_value array_with_true = json_array();
	json_array_add(&array_with_true, json_true);

	struct test {
		char *input;
		struct json_value want;
	} tests[] = {
		{ "1", json_number(1) },
		{ "11", json_number(11) },
		{ "\"hello\"", json_string("hello") },
		{ "\"\"", json_string("") },
		{ "true", json_true },
		{ "false", json_false },
		{ "null", json_null },
		{ "[]", json_array() },
		{ "{}", json_object() },
		{ "[true]", array_with_true },
		{ "{\"name\": \"David\", \"age\": 20}", make_person("David", 20) },
		{ "{\"age\": 20, \"name\": \"David\"}", make_person("David", 20) }
	};

	for (int i = 0; i < sizeof tests / sizeof tests[0]; i++) {
		struct test test = tests[i];

		printf("=== Test no. %i: '%s'--> <%s>\n",
			i + 1,
			test.input,
			json_stringify(test.want));

		struct json_parse_result got = json_parse(test.input);
		if (got.error != NULL) {
			printf("Err: '%s'; line %i col %i\n", got.error, got.line_number, got.column);
			continue;
		}

		if (!json_equal(test.want, got.value)) {
			printf("Not equal, got '%s'\n", json_stringify(got.value));
		}
	}
}
