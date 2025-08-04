struct json_value {
	enum json_value_type {
		json_nil
		json_true,
		json_false,
		json_number,
		json_string,
		json_array,
		json_object
	} type;
	union {
		const char *string;
		int number;
	};
};

struct json_parse_result {
	struct json_value value;
	char *error;
};

struct json_parse_result json_parse(const char *s);

char *json_value_stringify(struct json_value);
bool json_values_equal(struct json_value, struct json_value);

struct json_value json_value_number(int);
struct json_value json_value_string(const char *);
