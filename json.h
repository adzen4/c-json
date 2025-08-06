#pragma once

struct json_value {
	enum json_value_type {
		json_type_undefined,
		json_type_null,
		json_type_true,
		json_type_false,
		json_type_number,
		json_type_string,
		json_type_array,
		json_type_object,
	} type;
	union {
		char *string;
		int number;
		struct {
			int length;
			int max;
			union {
				struct json_value *children;
				struct json_key *keys;
			};
		};
	};
};

#define json_undefined (struct json_value) { .type = json_type_undefined }
#define json_null      (struct json_value) { .type = json_type_null }
#define json_true	   (struct json_value) { .type = json_type_true }
#define json_false     (struct json_value) { .type = json_type_false }

struct json_value json_number(int);
struct json_value json_string(char *);
struct json_value json_array(void);
struct json_value json_object(void);

void json_array_add(struct json_value *, struct json_value);
void json_object_add(struct json_value *, char *key, struct json_value);
struct json_value json_array_get(struct json_value, int index);
struct json_value json_object_get(struct json_value, char *key);

char *json_sprintf(char *fmt, ...);

bool json_equal(struct json_value, struct json_value);
char *json_stringify(struct json_value);

struct json_parse_result {
	struct json_value value;
	char *error;
	int line_number;
	int column;
};

struct json_parse_result json_parse(char *s);
void json_free(struct json_value);
