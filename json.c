#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>

struct json_value {
	enum json_value_type {
		json_string,
		json_number
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

struct json_parse_result json_parse(const char *s)
{
	enum {
		state_start,
		state_number,
		state_end
	} state;
	int number = 0;

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
				continue;
			}

			return error("Illegal character: '%c'", *s);

			break;
		case state_number:
			if (!isdigit(*s)) {
				state = state_end;
				continue;
			}

			number *= 10;
			number += (*s++ - '0');
			break;
		case state_end:
			if (*s == '\0') {
				goto done;
			}
			
			if (isspace(*s)) {
				s++;
				continue;
			}

			return error("Illegal character: '%c'", *s);
			
			break;
		default:
			assert(false);
		}
	}

done:
	return (struct json_parse_result) {
		.value = (struct json_value) {
			.type = json_number,
			.number = number
		},
	};
}

int main(void) {
	struct json_parse_result result;
	struct json_value value;
	
	result = json_parse("  \t\n");
	if (result.error != NULL) {
		printf("error: %s", result.error);
		return 1;
	}

	value = result.value;
	if (value.type != json_number) {
		printf("Wanted json type of number, got %i", value.type);
		return 1;
	}

	printf("json value is %i", value.number);
}
